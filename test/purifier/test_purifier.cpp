#include <iostream>
#include <stdexcept>
#include <type_traits>
#include "Appliance/AirConditioner/AirConditioner.h"

uint32_t test_millis = 100;
using namespace dudanov::midea;
using namespace dudanov::midea::ac;
#define REQUIRE(expr) do { if (!(expr)) throw std::runtime_error(#expr); } while (0)

class TestAC : public AirConditioner {
 public:
  using AirConditioner::m_readStatus;
  using AirConditioner::m_getStatus;
  void m_setup() override {}  // Keep the real base queue/timers, omit periodic queries.
  void m_onIdle() override {}
  bool cachedPurifier() const { return (this->m_status.data()[9] & 0x20) != 0; }
};

// On the original library, use its existing parser and notification path.
// The first RED is a missing notification for a real purifier-only status change,
// not an undeclared-API compilation error or a constant-false feature adapter.
template<class T, class = void> struct HasFeedback : std::false_type {};
template<class T> struct HasFeedback<T, std::void_t<decltype(
  std::declval<T>().addOnPurifierStatusCallback(std::function<void(bool)>{}))>> : std::true_type {};
template<class T> void feedback(T &ac, std::function<void(bool)> cb) {
  if constexpr (HasFeedback<T>::value)
    ac.addOnPurifierStatusCallback(cb);
  else
    ac.addOnStateCallback([&ac, cb]() { cb(ac.cachedPurifier()); });
}

FrameData statusFrame(bool purifier = false, uint8_t eco = 0) {
  // Entirely synthetic C0 body, including the identifier in byte numbering.
  std::vector<uint8_t> bytes(24, 0);
  bytes[0] = 0xC0; bytes[1] = 1; bytes[2] = 0x48; bytes[3] = 40;
  bytes[4] = bytes[5] = 0x7F; bytes[7] = 0x3C;
  bytes[9] = (purifier ? 0x20 : 0) | eco;
  bytes[11] = 84; bytes[12] = 72;
  FrameData data(bytes.data(), bytes.size());
  data.appendCRC();
  return data;
}

void remote_only_status_reaches_feedback() {
  TestAC ac;
  std::vector<bool> actual;
  feedback(ac, [&](bool state) { actual.push_back(state); });
  REQUIRE(ac.m_readStatus(statusFrame(false)) == RESPONSE_OK);
  REQUIRE(actual.size() == 1 && !actual.back());
  REQUIRE(ac.m_readStatus(statusFrame(true)) == RESPONSE_OK);
  REQUIRE(ac.cachedPurifier());
  REQUIRE(actual.size() == 2 && actual.back());
}

template<class T, class = void> struct HasBitSetter : std::false_type {};
template<class T> struct HasBitSetter<T, std::void_t<decltype(std::declval<T>().setPurifier(true))>> : std::true_type {};
template<class T> void setBit(T &status, bool state) {
  if constexpr (HasBitSetter<T>::value) status.setPurifier(state);
}
void purifier_bit_is_independent_of_eco() {
  StatusData status(statusFrame(true, 0x10));
  REQUIRE(status.getPreset() == PRESET_ECO);
  const auto before = status;
  setBit(status, false);
  REQUIRE((status.data()[9] & 0x20) == 0);
  for (unsigned i = 0; i < status.size(); i++)
    REQUIRE(status.data()[i] == (i == 9 ? (before.data()[i] & ~0x20) : before.data()[i]));
  setBit(status, true);
  REQUIRE(std::memcmp(status.data(), before.data(), status.size()) == 0);
}

class MemoryStream : public dudanov::Stream {
 public:
  std::deque<uint8_t> rx;
  std::vector<std::vector<uint8_t>> tx;
  int available() override { return rx.size(); }
  int read() override { if (rx.empty()) return -1; int b = rx.front(); rx.pop_front(); return b; }
  int peek() override { return rx.empty() ? -1 : rx.front(); }
  size_t write(uint8_t data) override { return write(&data, 1); }
  size_t write(const uint8_t *data, size_t size) override { tx.emplace_back(data, data + size); return size; }
  void flush() override {}
  void receive(const FrameData &data, FrameType type = DEVICE_CONTROL) {
    Frame frame(AIR_CONDITIONER, 2, type, data);
    rx.insert(rx.end(), frame.data(), frame.data() + frame.size());
  }
};
struct Fixture {
  MemoryStream stream;
  TestAC ac;
  Fixture() {
    test_millis += 100;
    dudanov::TimerManager clock; clock.task();
    ac.setStream(&stream); ac.setPeriod(1); ac.setTimeout(10); ac.setNumAttempts(1);
    ac.setup(); ac.loop(); test_millis += 2; ac.loop(); stream.tx.clear();
  }
};
template<class T, class = void> struct HasControl : std::false_type {};
template<class T> struct HasControl<T, std::void_t<decltype(
  std::declval<T>().setPurifier(true, uint32_t(15000)))>> : std::true_type {};
template<class T> bool requestPurifier(T &ac, bool state, uint32_t age = 15000) {
  if constexpr (HasControl<T>::value) return ac.setPurifier(state, age);
  else return false;
}
void queued_command_is_feedback_driven() {
  Fixture f;
  std::vector<bool> actual;
  feedback(f.ac, [&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true));
  REQUIRE(!f.ac.getPurifier());
  REQUIRE(actual.size() == 1 && !actual.back());
  f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  const auto &tx = f.stream.tx[0];
  REQUIRE(tx[9] == DEVICE_CONTROL && tx[10] == 0x40);
  REQUIRE((tx[19] & 0x20) != 0);
  FrameData body(tx.data() + 10, tx.size() - 11);
  REQUIRE(body.hasValidCRC());
  unsigned sum = 0; for (unsigned i = 1; i < tx.size(); i++) sum += tx[i];
  REQUIRE((sum & 255) == 0);
  REQUIRE(actual.size() == 1);
  f.stream.receive(statusFrame(true)); f.ac.loop();
  REQUIRE(f.ac.getPurifier());
  REQUIRE(actual.size() == 2 && actual.back());
}

void preserves_climate_fields_and_preset_encoding() {
  for (bool powered : {false, true}) {
    for (Preset preset : {PRESET_NONE, PRESET_ECO, PRESET_TURBO, PRESET_SLEEP, PRESET_FREEZE_PROTECTION}) {
      Fixture f;
      std::vector<uint8_t> bytes(24, 0);
      bytes[0] = 0xC0; bytes[1] = powered ? 1 : 0;
      bytes[2] = 0x55; bytes[3] = 50;  // raw fan must NOT normalize to 60 or AUTO
      bytes[4] = bytes[5] = 0x7F; bytes[7] = 0x3F;
      bytes[11] = 84; bytes[12] = 72; bytes[13] = 15;  // extended target 27.5C
      if (preset == PRESET_ECO) bytes[9] = 0x10;
      if (preset == PRESET_TURBO) bytes[8] = 0x20;
      if (preset == PRESET_SLEEP) bytes[10] = 1;
      if (preset == PRESET_FREEZE_PROTECTION) bytes[21] = 0x80;
      FrameData data(bytes.data(), bytes.size()); data.appendCRC();
      f.ac.m_readStatus(data);
      REQUIRE(f.ac.getPreset() == preset);
      REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
      REQUIRE(f.stream.tx.size() == 1);
      const auto &tx = f.stream.tx[0];
      REQUIRE((tx[19] & 0x80) == (preset == PRESET_ECO ? 0x80 : 0));
      REQUIRE(tx[11] == (bytes[1] | 2));  // normal beeper-control mask only
      REQUIRE((tx[12] & 0xE0) == (bytes[2] & 0xE0));
      REQUIRE(tx[13] == bytes[3]);
      REQUIRE(tx[17] == bytes[7]);
      REQUIRE(tx[28] == 15);  // extended target in command byte18, not status byte13
      REQUIRE((tx[12] & 0x1F) == 0x1B);
      REQUIRE((tx[31] & 0x80) == (preset == PRESET_FREEZE_PROTECTION ? 0x80 : 0));
      REQUIRE((tx[18] & 0x20) == (preset == PRESET_TURBO ? 0x20 : 0));
      REQUIRE((tx[20] & 1) == (preset == PRESET_SLEEP ? 1 : 0));
      f.stream.receive(data); f.ac.loop();
    }
  }
}

template<class T, class = void> struct HasFreshness : std::false_type {};
template<class T> struct HasFreshness<T, std::void_t<decltype(
  std::declval<T>().hasFreshStatus(uint32_t(15000)))>> : std::true_type {};
template<class T> bool fresh(const T &ac, uint32_t age = 15000) {
  if constexpr (HasFreshness<T>::value) return ac.hasFreshStatus(age);
  else return true;
}
void requires_fresh_confirmed_status() {
  Fixture f;
  REQUIRE(!requestPurifier(f.ac, true));
  REQUIRE(!fresh(f.ac)); f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(fresh(f.ac)); REQUIRE(!fresh(f.ac, 0));
  REQUIRE(!requestPurifier(f.ac, true, 0));
  test_millis += 15000;
  REQUIRE(fresh(f.ac));
  test_millis++;
  REQUIRE(!fresh(f.ac));
  REQUIRE(!requestPurifier(f.ac, true));
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void rejects_duplicates_and_busy_climate_writes() {
  Fixture f;
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(!requestPurifier(f.ac, false)); f.ac.loop(); REQUIRE(f.stream.tx.empty());
  REQUIRE(requestPurifier(f.ac, true));
  REQUIRE(!requestPurifier(f.ac, true));
  REQUIRE(!requestPurifier(f.ac, false));
  Control control; control.targetTemp = 25.0f; f.ac.control(control);
  f.ac.loop(); REQUIRE(f.stream.tx.size() == 1);
  f.stream.receive(statusFrame(true)); f.ac.loop(); test_millis += 2; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);  // climate command did not queue while purifier was busy
  f.ac.control(control);
  REQUIRE(!requestPurifier(f.ac, false));
  f.ac.loop(); REQUIRE(f.stream.tx.size() == 2);
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void malformed_status_cannot_refresh_or_publish() {
  Fixture f;
  std::vector<bool> actual;
  feedback(f.ac, [&](bool state) { actual.push_back(state); });
  auto good = statusFrame(true);
  std::vector<uint8_t> bad(good.data(), good.data() + good.size());
  bad.back() ^= 1;
  // Existing climate parsing is intentionally unchanged for complete replies.
  REQUIRE(f.ac.m_readStatus(FrameData(bad.data(), bad.size())) == RESPONSE_OK);
  REQUIRE(f.ac.m_readStatus(FrameData({0xC0})) == RESPONSE_WRONG);
  REQUIRE(f.ac.m_readStatus(FrameData(uint8_t(0))) == RESPONSE_WRONG);
  FrameData shortBody({0xC0, 1, 0x48, 40, 0x7F, 0x7F, 0, 0x3C, 0, 0x20, 0});
  shortBody.appendCRC();
  REQUIRE(f.ac.m_readStatus(shortBody) == RESPONSE_OK);
  REQUIRE(actual.empty() && !fresh(f.ac));
  REQUIRE(!requestPurifier(f.ac, true));
  f.ac.m_readStatus(good);
  REQUIRE(actual.size() == 1 && actual.back());
  // A later malformed status must revoke even a still-fresh write snapshot.
  test_millis++;
  REQUIRE(f.ac.m_readStatus(FrameData(bad.data(), bad.size())) == RESPONSE_OK);
  REQUIRE(!fresh(f.ac));
  REQUIRE(!requestPurifier(f.ac, false));
  REQUIRE(actual.size() == 1 && f.ac.getPurifier());
}

void all_callbacks_see_complete_confirmed_status() {
  Fixture f;
  unsigned climateUpdates = 0, purifierUpdates = 0;
  f.ac.addOnStateCallback([&]() {
    climateUpdates++;
    REQUIRE(f.ac.getPurifier());
    REQUIRE(f.ac.hasFreshStatus());
    REQUIRE(f.ac.getMode() == MODE_COOL);
  });
  feedback(f.ac, [&](bool state) {
    purifierUpdates++;
    REQUIRE(state && f.ac.getMode() == MODE_COOL);
  });
  f.ac.m_readStatus(statusFrame(true));
  REQUIRE(climateUpdates == 1 && purifierUpdates == 1);
  f.ac.m_readStatus(statusFrame(true));
  REQUIRE(climateUpdates == 1 && purifierUpdates == 2);
}

void timeout_and_opposite_response_do_not_publish_request() {
  Fixture f;
  std::vector<bool> actual;
  feedback(f.ac, [&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  test_millis += 11; f.ac.loop();
  REQUIRE(actual.size() == 1 && !f.ac.getPurifier());
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  f.stream.receive(statusFrame(false)); f.ac.loop();
  REQUIRE(actual.size() == 2 && !actual.back() && !f.ac.getPurifier());
  test_millis += 2; f.ac.loop();
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  f.stream.receive(statusFrame(true)); f.ac.loop();
  REQUIRE(actual.size() == 3 && actual.back());
  test_millis += 2; f.ac.loop();
  REQUIRE(requestPurifier(f.ac, false)); f.ac.loop();
  REQUIRE((f.stream.tx.back()[19] & 0x20) == 0);
  REQUIRE(f.ac.getPurifier());
  f.stream.receive(statusFrame(false)); f.ac.loop();
  REQUIRE(actual.size() == 4 && !actual.back());
}

void polled_remote_changes_do_not_replace_climate_callbacks() {
  Fixture f;
  unsigned climateUpdates = 0;
  std::vector<bool> actual;
  f.ac.addOnStateCallback([&]() { climateUpdates++; });
  feedback(f.ac, [&](bool state) { actual.push_back(state); });
  for (bool state : {false, true, false}) {
    f.ac.m_getStatus(); f.ac.loop();
    f.stream.receive(statusFrame(state), DEVICE_QUERY); f.ac.loop();
    test_millis += 2; f.ac.loop();
    REQUIRE(f.ac.getPurifier() == state);
  }
  REQUIRE(actual.size() == 3 && !actual[0] && actual[1] && !actual[2]);
  REQUIRE(climateUpdates == 1);
}

void startup_and_stale_recovery_never_restore_purifier() {
  MemoryStream stream;
  AirConditioner ac;
  ac.setStream(&stream); ac.setAutoconf(false); ac.setPeriod(1);
  ac.setTimeout(10); ac.setNumAttempts(1);
  ac.setup();
  for (unsigned i = 0; i < 12; i++) { test_millis += 2; ac.loop(); }
  REQUIRE(!ac.hasFreshStatus());
  REQUIRE(!ac.setPurifier(true));
  for (const auto &tx : stream.tx) REQUIRE(tx[9] != DEVICE_CONTROL);
  Fixture f;
  f.ac.m_readStatus(statusFrame(true));
  test_millis += 15001;
  REQUIRE(!requestPurifier(f.ac, false)); f.ac.loop();
  REQUIRE(f.stream.tx.empty());
  f.ac.m_getStatus(); f.ac.loop();
  f.stream.receive(statusFrame(false), DEVICE_QUERY); f.ac.loop();
  REQUIRE(!f.ac.getPurifier() && fresh(f.ac));
  for (const auto &tx : f.stream.tx) REQUIRE(tx[9] != DEVICE_CONTROL);
}

void freshness_handles_millisecond_rollover() {
  TestAC ac;
  test_millis = UINT32_MAX - 10;
  ac.m_readStatus(statusFrame(false));
  test_millis = 9;
  REQUIRE(ac.hasFreshStatus(20));
  REQUIRE(!ac.hasFreshStatus(19));
  test_millis = 100;
}

void short_and_legacy_status_are_feature_compatible() {
  for (unsigned length : {16, 19, 24}) {
    for (bool legacy : {false, true}) {
      Fixture f;
      std::vector<bool> actual;
      feedback(f.ac, [&](bool state) { actual.push_back(state); });
      auto full = statusFrame(true, 0x10);
      std::vector<uint8_t> bytes(full.data(), full.data() + length);
      FrameData data(bytes.data(), bytes.size());
      if (legacy) {
        uint8_t checksum = 0;
        for (uint8_t byte : bytes) checksum -= byte;
        bytes.push_back(checksum);
        data = FrameData(bytes.data(), bytes.size());
      } else {
        data.appendCRC();
      }
      REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
      REQUIRE(fresh(f.ac) && f.ac.getPurifier());
      REQUIRE(actual.size() == 1 && actual.back());
      REQUIRE(requestPurifier(f.ac, false)); f.ac.loop();
      REQUIRE((f.stream.tx.back()[19] & 0x20) == 0);
      REQUIRE((f.stream.tx.back()[19] & 0x80) != 0);
      f.stream.receive(data); f.ac.loop();
    }
  }
}

void queued_write_is_cancelled_if_status_becomes_untrustworthy() {
  Fixture f;
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true));
  auto good = statusFrame(false);
  std::vector<uint8_t> bad(good.data(), good.data() + good.size());
  bad.back() ^= 1;
  f.ac.m_readStatus(FrameData(bad.data(), bad.size()));
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(good);
  REQUIRE(requestPurifier(f.ac, true, 5));
  test_millis += 6;
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(good);
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void retries_do_not_send_an_expired_snapshot() {
  Fixture f;
  f.ac.setNumAttempts(3);
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true, 5)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  test_millis += 11; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  REQUIRE(!f.ac.getPurifier());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void integrity_byte_is_not_an_optional_preset() {
  Fixture f;
  auto full = statusFrame(false);
  std::vector<uint8_t> bytes(full.data(), full.data() + 21);
  uint8_t sum = 0;
  for (uint8_t byte : bytes) sum += byte;
  bytes[20] += uint8_t(0x80 - sum);
  bytes.push_back(0x80);  // Valid additive checksum at status index21, NOT freeze protection.
  FrameData data(bytes.data(), bytes.size());
  REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  REQUIRE((f.stream.tx.back()[31] & 0x80) == 0);
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void changed_snapshot_cancels_pending_write() {
  Fixture f;
  f.ac.m_getStatus(); f.ac.loop();
  f.stream.receive(statusFrame(false), DEVICE_QUERY); f.ac.loop();
  REQUIRE(requestPurifier(f.ac, true));
  // A new confirmation supersedes the queued snapshot, even if the bit is unchanged.
  f.ac.m_readStatus(statusFrame(false));
  test_millis += 2; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1 && f.stream.tx[0][9] == DEVICE_QUERY);
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 2);
  f.stream.receive(statusFrame(true)); f.ac.loop();
}

void malformed_uart_ack_revokes_write_permission() {
  Fixture f;
  std::vector<bool> actual;
  feedback(f.ac, [&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(requestPurifier(f.ac, true)); f.ac.loop();
  auto good = statusFrame(true);
  std::vector<uint8_t> bad(good.data(), good.data() + good.size());
  bad.back() ^= 1;
  // Real outer frame checksum passes; the feature-local body integrity gate must fail.
  f.stream.receive(FrameData(bad.data(), bad.size())); f.ac.loop();
  REQUIRE(actual.size() == 1 && !f.ac.getPurifier());
  REQUIRE(!f.ac.hasFreshStatus());
  REQUIRE(!requestPurifier(f.ac, true));
  f.ac.m_getStatus(); test_millis += 2; f.ac.loop();
  f.stream.receive(statusFrame(true), DEVICE_QUERY); f.ac.loop();
  REQUIRE(actual.size() == 2 && actual.back() && f.ac.hasFreshStatus());
}

using Test = std::pair<const char *, void(*)()>;
int main(int argc, char **argv) {
  const Test tests[] = {
    {"malformed_uart_ack_revokes_write_permission", malformed_uart_ack_revokes_write_permission},
    {"changed_snapshot_cancels_pending_write", changed_snapshot_cancels_pending_write},
    {"integrity_byte_is_not_an_optional_preset", integrity_byte_is_not_an_optional_preset},
    {"retries_do_not_send_an_expired_snapshot", retries_do_not_send_an_expired_snapshot},
    {"queued_write_is_cancelled_if_status_becomes_untrustworthy", queued_write_is_cancelled_if_status_becomes_untrustworthy},
    {"short_and_legacy_status_are_feature_compatible", short_and_legacy_status_are_feature_compatible},
    {"freshness_handles_millisecond_rollover", freshness_handles_millisecond_rollover},
    {"startup_and_stale_recovery_never_restore_purifier", startup_and_stale_recovery_never_restore_purifier},
    {"polled_remote_changes_do_not_replace_climate_callbacks", polled_remote_changes_do_not_replace_climate_callbacks},
    {"timeout_and_opposite_response_do_not_publish_request", timeout_and_opposite_response_do_not_publish_request},
    {"all_callbacks_see_complete_confirmed_status", all_callbacks_see_complete_confirmed_status},
    {"malformed_status_cannot_refresh_or_publish", malformed_status_cannot_refresh_or_publish},
    {"rejects_duplicates_and_busy_climate_writes", rejects_duplicates_and_busy_climate_writes},
    {"requires_fresh_confirmed_status", requires_fresh_confirmed_status},
    {"preserves_climate_fields_and_preset_encoding", preserves_climate_fields_and_preset_encoding},
    {"queued_command_is_feedback_driven", queued_command_is_feedback_driven},
    {"purifier_bit_is_independent_of_eco", purifier_bit_is_independent_of_eco},
    {"remote_only_status_reaches_feedback", remote_only_status_reaches_feedback},
  };
  unsigned failures = 0, selected = 0;
  for (const auto &test : tests) {
    if (argc > 1 && std::string(argv[1]) != test.first) continue;
    selected++;
    try { test.second(); std::cout << "PASS " << test.first << '\n'; }
    catch (const std::exception &e) {
      failures++; std::cerr << "FAIL " << test.first << ": " << e.what() << '\n';
    }
  }
  if (!selected) { std::cerr << "No matching test\n"; return 2; }
  std::cout << selected << " tests, " << failures << " failures\n";
  return failures ? 1 : 0;
}
