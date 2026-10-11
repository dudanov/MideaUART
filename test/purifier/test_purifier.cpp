#include <iostream>
#include <stdexcept>
#include <type_traits>
#include "Appliance/AirConditioner/AirConditioner.h"

uint32_t test_millis = 100;
using namespace dudanov::midea;
using namespace dudanov::midea::ac;
#define REQUIRE(expr) do { if (!(expr)) throw std::runtime_error(#expr); } while (0)

static_assert(std::is_same_v<decltype(&AirConditioner::setPurifier),
  bool (AirConditioner::*)(bool, uint32_t)>);
static_assert(std::is_same_v<decltype(&AirConditioner::getPurifier),
  bool (AirConditioner::*)() const>);
static_assert(std::is_same_v<decltype(&AirConditioner::hasFreshStatus),
  bool (AirConditioner::*)(uint32_t) const>);
static_assert(std::is_same_v<decltype(&AirConditioner::addOnPurifierStatusCallback),
  void (AirConditioner::*)(OnPurifierStatusCallback)>);

class TestAC : public AirConditioner {
 public:
  using AirConditioner::m_readStatus;
  using AirConditioner::m_getStatus;
  void m_setup() override {}  // Keep the real base queue/timers, omit periodic queries.
  void m_onIdle() override {}
};

std::vector<uint8_t> statusBytes(bool purifier = false, uint8_t eco = 0, unsigned size = 24) {
  // Synthetic C0 body; offsets include the identifier.
  std::vector<uint8_t> bytes(24, 0);
  bytes[0] = 0xC0; bytes[1] = 1; bytes[2] = 0x48; bytes[3] = 40;
  bytes[4] = bytes[5] = 0x7F; bytes[7] = 0x3C;
  bytes[9] = (purifier ? 0x20 : 0) | eco;
  bytes[11] = 84; bytes[12] = 72;
  bytes.resize(size);
  return bytes;
}

FrameData statusReply(std::vector<uint8_t> bytes, bool legacy = false) {
  if (legacy) {
    uint8_t checksum = 0;
    for (uint8_t byte : bytes) checksum -= byte;
    bytes.push_back(checksum);
    return FrameData(bytes.data(), bytes.size());
  }
  FrameData data(bytes.data(), bytes.size());
  data.appendCRC();
  return data;
}

FrameData statusFrame(bool purifier = false, uint8_t eco = 0) {
  return statusReply(statusBytes(purifier, eco));
}

FrameData corruptCRC(const FrameData &data) {
  std::vector<uint8_t> bytes(data.data(), data.data() + data.size());
  bytes.back() ^= 1;
  return FrameData(bytes.data(), bytes.size());
}

void remote_only_status_reaches_feedback() {
  TestAC ac;
  std::vector<bool> actual;
  ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  REQUIRE(ac.m_readStatus(statusFrame(false)) == RESPONSE_OK);
  REQUIRE(actual.size() == 1 && !actual.back());
  REQUIRE(ac.m_readStatus(statusFrame(true)) == RESPONSE_OK);
  REQUIRE(ac.getPurifier());
  REQUIRE(actual.size() == 2 && actual.back());
}

void purifier_bit_is_independent_of_eco() {
  StatusData status(statusFrame(true, 0x10));
  REQUIRE(status.getPreset() == PRESET_ECO);
  const auto before = status;
  status.setPurifier(false);
  REQUIRE((status.data()[9] & 0x20) == 0);
  for (unsigned i = 0; i < status.size(); i++)
    REQUIRE(status.data()[i] == (i == 9 ? (before.data()[i] & ~0x20) : before.data()[i]));
  status.setPurifier(true);
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
  void receive(const FrameData &data, FrameType type = DEVICE_CONTROL) {
    stream.receive(data, type);
    ac.loop();
  }
};

void queued_command_is_feedback_driven() {
  Fixture f;
  std::vector<bool> actual;
  f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true));
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
  f.receive(statusFrame(true));
  REQUIRE(f.ac.getPurifier());
  REQUIRE(actual.size() == 2 && actual.back());
}

void preserves_climate_fields_and_preset_encoding() {
  for (bool powered : {false, true}) {
    for (Preset preset : {PRESET_NONE, PRESET_ECO, PRESET_TURBO, PRESET_SLEEP, PRESET_FREEZE_PROTECTION}) {
      Fixture f;
      auto bytes = statusBytes();
      bytes[1] = powered ? 1 : 0;
      bytes[2] = 0x55; bytes[3] = 50;  // Keep raw fan speed, not 60 or AUTO.
      bytes[7] = 0x3F; bytes[13] = 15;  // Extended target 27.5C.
      if (preset == PRESET_ECO) bytes[9] = 0x10;
      if (preset == PRESET_TURBO) bytes[8] = 0x20;
      if (preset == PRESET_SLEEP) bytes[10] = 1;
      if (preset == PRESET_FREEZE_PROTECTION) bytes[21] = 0x80;
      auto data = statusReply(bytes);
      f.ac.m_readStatus(data);
      REQUIRE(f.ac.getPreset() == preset);
      REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
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
      f.receive(data);
    }
  }
}

void requires_fresh_confirmed_status() {
  Fixture f;
  REQUIRE(!f.ac.setPurifier(true));
  REQUIRE(!f.ac.hasFreshStatus()); f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.hasFreshStatus()); REQUIRE(!f.ac.hasFreshStatus(0));
  REQUIRE(!f.ac.setPurifier(true, 0));
  test_millis += 15000;
  REQUIRE(f.ac.hasFreshStatus());
  test_millis++;
  REQUIRE(!f.ac.hasFreshStatus());
  REQUIRE(!f.ac.setPurifier(true));
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(statusFrame(true));
}

void rejects_duplicates_and_busy_climate_writes() {
  Fixture f;
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(!f.ac.setPurifier(false)); f.ac.loop(); REQUIRE(f.stream.tx.empty());
  REQUIRE(f.ac.setPurifier(true));
  REQUIRE(!f.ac.setPurifier(true));
  REQUIRE(!f.ac.setPurifier(false));
  Control control; control.targetTemp = 25.0f; f.ac.control(control);
  f.ac.loop(); REQUIRE(f.stream.tx.size() == 1);
  f.receive(statusFrame(true)); test_millis += 2; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);  // climate command did not queue while purifier was busy
  f.ac.control(control);
  REQUIRE(!f.ac.setPurifier(false));
  f.ac.loop(); REQUIRE(f.stream.tx.size() == 2);
  f.receive(statusFrame(true));
}

void malformed_status_cannot_refresh_or_publish() {
  Fixture f;
  std::vector<bool> actual;
  f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  auto good = statusFrame(true);
  auto bad = corruptCRC(good);
  // Existing climate parsing is intentionally unchanged for complete replies.
  REQUIRE(f.ac.m_readStatus(bad) == RESPONSE_OK);
  REQUIRE(f.ac.m_readStatus(FrameData({0xC0})) == RESPONSE_WRONG);
  REQUIRE(f.ac.m_readStatus(FrameData(uint8_t(0))) == RESPONSE_WRONG);
  FrameData shortBody({0xC0, 1, 0x48, 40, 0x7F, 0x7F, 0, 0x3C, 0, 0x20, 0});
  // The legacy climate minimum remains accepted; integrity-free getters must
  // not shorten the source of copyStatus() into an out-of-bounds prefix copy.
  REQUIRE(f.ac.m_readStatus(shortBody) == RESPONSE_OK);
  REQUIRE(actual.empty() && !f.ac.hasFreshStatus());
  shortBody.appendCRC();
  REQUIRE(f.ac.m_readStatus(shortBody) == RESPONSE_OK);
  REQUIRE(actual.empty() && !f.ac.hasFreshStatus());
  REQUIRE(!f.ac.setPurifier(true));
  f.ac.m_readStatus(good);
  REQUIRE(actual.size() == 1 && actual.back());
  // A later malformed status must revoke even a still-fresh write snapshot.
  test_millis++;
  REQUIRE(f.ac.m_readStatus(bad) == RESPONSE_OK);
  REQUIRE(!f.ac.hasFreshStatus());
  REQUIRE(!f.ac.setPurifier(false));
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
  f.ac.addOnPurifierStatusCallback([&](bool state) {
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
  f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  test_millis += 11; f.ac.loop();
  REQUIRE(actual.size() == 1 && !f.ac.getPurifier());
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(statusFrame(false));
  REQUIRE(actual.size() == 2 && !actual.back() && !f.ac.getPurifier());
  test_millis += 2; f.ac.loop();
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(statusFrame(true));
  REQUIRE(actual.size() == 3 && actual.back());
  test_millis += 2; f.ac.loop();
  REQUIRE(f.ac.setPurifier(false)); f.ac.loop();
  REQUIRE((f.stream.tx.back()[19] & 0x20) == 0);
  REQUIRE(f.ac.getPurifier());
  f.receive(statusFrame(false));
  REQUIRE(actual.size() == 4 && !actual.back());
}

void polled_remote_changes_do_not_replace_climate_callbacks() {
  Fixture f;
  unsigned climateUpdates = 0;
  std::vector<bool> actual;
  f.ac.addOnStateCallback([&]() { climateUpdates++; });
  f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  for (bool state : {false, true, false}) {
    f.ac.m_getStatus(); f.ac.loop();
    f.receive(statusFrame(state), DEVICE_QUERY);
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
  REQUIRE(!f.ac.setPurifier(false)); f.ac.loop();
  REQUIRE(f.stream.tx.empty());
  f.ac.m_getStatus(); f.ac.loop();
  f.receive(statusFrame(false), DEVICE_QUERY);
  REQUIRE(!f.ac.getPurifier() && f.ac.hasFreshStatus());
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
      f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
      auto data = statusReply(statusBytes(true, 0x10, length), legacy);
      REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
      REQUIRE(f.ac.hasFreshStatus() && f.ac.getPurifier());
      REQUIRE(actual.size() == 1 && actual.back());
      REQUIRE(f.ac.setPurifier(false)); f.ac.loop();
      REQUIRE((f.stream.tx.back()[19] & 0x20) == 0);
      REQUIRE((f.stream.tx.back()[19] & 0x80) != 0);
      f.receive(data);
    }
  }
}

void queued_write_is_cancelled_if_status_becomes_untrustworthy() {
  Fixture f;
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true));
  auto good = statusFrame(false);
  auto bad = corruptCRC(good);
  f.ac.m_readStatus(bad);
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(good);
  REQUIRE(f.ac.setPurifier(true, 5));
  test_millis += 6;
  f.ac.loop(); REQUIRE(f.stream.tx.empty());
  f.ac.m_readStatus(good);
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  f.receive(statusFrame(true));
}

void retries_do_not_send_an_expired_snapshot() {
  Fixture f;
  f.ac.setNumAttempts(3);
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true, 5)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  test_millis += 11; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);
  REQUIRE(!f.ac.getPurifier());
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(statusFrame(true));
}

void integrity_byte_is_not_an_optional_preset() {
  Fixture f;
  auto bytes = statusBytes(false, 0, 21);
  uint8_t sum = 0;
  for (uint8_t byte : bytes) sum += byte;
  bytes[20] += uint8_t(0x80 - sum);
  auto data = statusReply(bytes, true);
  REQUIRE(data.data()[21] == 0x80);  // Checksum, not freeze protection.
  REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  REQUIRE((f.stream.tx.back()[31] & 0x80) == 0);
  f.receive(statusFrame(true));
}

void changed_snapshot_cancels_pending_write() {
  Fixture f;
  f.ac.m_getStatus(); f.ac.loop();
  f.receive(statusFrame(false), DEVICE_QUERY);
  REQUIRE(f.ac.setPurifier(true));
  // A new confirmation supersedes the queued snapshot, even if the bit is unchanged.
  f.ac.m_readStatus(statusFrame(false));
  test_millis += 2; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1 && f.stream.tx[0][9] == DEVICE_QUERY);
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  REQUIRE(f.stream.tx.size() == 2);
  f.receive(statusFrame(true));
}

void malformed_uart_ack_revokes_write_permission() {
  Fixture f;
  std::vector<bool> actual;
  f.ac.addOnPurifierStatusCallback([&](bool state) { actual.push_back(state); });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  auto good = statusFrame(true);
  auto bad = corruptCRC(good);
  // Real outer frame checksum passes; the feature-local body integrity gate must fail.
  f.receive(bad);
  REQUIRE(actual.size() == 1 && !f.ac.getPurifier());
  REQUIRE(!f.ac.hasFreshStatus());
  REQUIRE(!f.ac.setPurifier(true));
  f.ac.m_getStatus(); test_millis += 2; f.ac.loop();
  f.receive(statusFrame(true), DEVICE_QUERY);
  REQUIRE(actual.size() == 2 && actual.back() && f.ac.hasFreshStatus());
}

void purifier_command_does_not_echo_response_bits() {
  for (bool powered : {false, true}) {
    for (uint8_t rawFan : {30, 50}) {
      Fixture f;
      std::vector<uint8_t> bytes(24, 0);
      bytes[0] = 0xC0; bytes[1] = 0x80 | powered;
      bytes[2] = 0x55; bytes[3] = rawFan;
      bytes[4] = 0x12; bytes[5] = 0x34; bytes[6] = 0xFF;
      bytes[7] = 0xBF; bytes[8] = 0x01; bytes[9] = 0x5F;
      bytes[10] = 0xFC; bytes[11] = 84; bytes[12] = 72; bytes[13] = 15;
      auto data = statusReply(bytes);
      REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
      REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
      REQUIRE(f.stream.tx.size() == 1);
      const auto &tx = f.stream.tx.back();
      std::vector<uint8_t> expected(24, 0);
      expected[0] = 0x40; expected[1] = powered ? 3 : 2;
      expected[2] = 0x5B; expected[3] = rawFan;
      expected[4] = expected[5] = 0x7F; expected[7] = 0x3F;
      expected[9] = 0xA0; expected[10] = 4; expected[18] = 15;
      REQUIRE(std::equal(expected.begin(), expected.end(), tx.begin() + 10));
      f.receive(statusFrame(true));
    }
  }
}

void purifier_toggle_preserves_independent_preset_flags() {
  // ECO+SLEEP in COOL, TURBO+SLEEP in COOL, and FREEZE+SLEEP in HEAT.
  for (uint8_t flags : {5, 6, 12}) {
    for (bool alternateTurbo : {false, true}) {
      Fixture f;
      std::vector<uint8_t> bytes(24, 0);
      bytes[0] = 0xC0; bytes[1] = 1;
      bytes[2] = flags == 12 ? 0x88 : 0x48; bytes[3] = 30; bytes[7] = 0x8C;
      bytes[9] = (flags & 1) ? 0x10 : 0;
      bytes[8] = (flags & 2) && !alternateTurbo ? 0x20 : 0;
      bytes[10] = ((flags & 4) ? 1 : 0) | ((flags & 2) && alternateTurbo ? 2 : 0);
      bytes[21] = (flags & 8) ? 0x80 : 0;
      auto data = statusReply(bytes);
      REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
      REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
      const auto &tx = f.stream.tx.back();
      REQUIRE(tx[13] == 30);  // feature-only toggle cannot apply preset fan constraints
      REQUIRE((tx[19] & 0x80) == ((flags & 1) ? 0x80 : 0));
      REQUIRE((tx[18] & 0x20) == ((flags & 2) ? 0x20 : 0));
      REQUIRE((tx[20] & 2) == ((flags & 2) ? 2 : 0));
      REQUIRE((tx[20] & 1) == ((flags & 4) ? 1 : 0));
      REQUIRE((tx[31] & 0x80) == ((flags & 8) ? 0x80 : 0));
      f.receive(statusFrame(true));
    }
  }
}

void nonstatus_uart_reply_preserves_guarded_retry() {
  for (const FrameData &reply : {FrameData({0xFF, 0x00}),  // busy body
                                 FrameData({0x40, 0x00}),  // short ACK, not C0
                                 FrameData(uint8_t(0)),   // empty nonstatus body
                                 FrameData({0xB5, 0x00, 0x00})}) {
    Fixture f;
    f.ac.setNumAttempts(3);
    unsigned climateUpdates = 0, purifierUpdates = 0;
    f.ac.addOnStateCallback([&]() { climateUpdates++; });
    f.ac.addOnPurifierStatusCallback([&](bool) { purifierUpdates++; });
    REQUIRE(f.ac.m_readStatus(statusFrame(false)) == RESPONSE_OK);
    REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
    REQUIRE(f.stream.tx.size() == 1);
    const auto original = f.stream.tx.back();
    test_millis += 4;
    f.receive(reply);
    REQUIRE(f.ac.hasFreshStatus());
    REQUIRE(!f.ac.getPurifier());
    REQUIRE(climateUpdates == 1 && purifierUpdates == 1);
    // Actual source classifies these as WRONG, not PARTIAL: the original
    // response deadline must survive, rather than being reset by busy/ACK.
    test_millis += 7; f.ac.loop();
    REQUIRE(f.stream.tx.size() == 2);
    const auto &retry = f.stream.tx.back();
    REQUIRE(retry.size() == original.size() && retry[9] == DEVICE_CONTROL);
    // Receiving a frame learns protocol 2; only the SET body must be identical.
    REQUIRE(std::equal(original.begin() + 10, original.end() - 1, retry.begin() + 10));
    f.receive(statusFrame(true));
    REQUIRE(f.ac.getPurifier() && purifierUpdates == 2);
  }
}

void truncated_c0_uart_reply_revokes_guarded_retry() {
  Fixture f;
  f.ac.setNumAttempts(3);
  unsigned purifierUpdates = 0;
  f.ac.addOnPurifierStatusCallback([&](bool) { purifierUpdates++; });
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(FrameData({0xC0, 0x01}));
  REQUIRE(!f.ac.hasFreshStatus() && !f.ac.getPurifier());
  REQUIRE(purifierUpdates == 1);
  test_millis += 11; f.ac.loop();
  REQUIRE(f.stream.tx.size() == 1);  // invalid snapshot cannot be retried
  REQUIRE(!f.ac.setPurifier(true));
  f.ac.m_readStatus(statusFrame(false));
  REQUIRE(f.ac.setPurifier(true)); f.ac.loop();
  f.receive(statusFrame(true));
}

void seventeen_byte_payload_has_no_phantom_humidity() {
  // Qualify the reported 18-byte CRC8 C0 against the actual bounds-checking reader.
  for (uint8_t suffix = 14; suffix <= 31; suffix++) {
    TestAC ac;
    auto bytes = statusBytes(false, 0, 17);
    bytes[16] = suffix;
    auto data = statusReply(bytes);
    REQUIRE(data.size() == 18 && data.hasValidCRC());
    REQUIRE(ac.m_readStatus(data) == RESPONSE_OK);
    REQUIRE(ac.hasFreshStatus());
    REQUIRE(ac.getIndoorHum() == 0);
    REQUIRE(ac.getIndoorTemp() == 17 && ac.getOutdoorTemp() == 11);
  }
}

void climate_optional_fields_exclude_reply_integrity() {
  for (unsigned payloadSize : {19, 21, 24}) {
    for (bool legacy : {false, true}) {
      Fixture f;
      auto bytes = statusBytes(false, 0, payloadSize);
      if (payloadSize == 24) { bytes[19] = 45; bytes[21] = 0x80; }
      // Force a nonzero integrity byte at absent humidity/freeze field offsets.
      if (legacy) {
        uint8_t sum = 0; for (uint8_t byte : bytes) sum += byte;
        bytes[16] += uint8_t(0x7F - sum);  // additive integrity byte becomes 0x81
      }
      auto data = statusReply(bytes, legacy);
      REQUIRE(data.data()[payloadSize] != 0);
      unsigned climateUpdates = 0;
      f.ac.addOnStateCallback([&]() {
        climateUpdates++;
        REQUIRE(f.ac.getIndoorHum() == (payloadSize == 24 ? 45 : 0));
        REQUIRE(f.ac.getPreset() == (payloadSize == 24 ? PRESET_FREEZE_PROTECTION : PRESET_NONE));
        REQUIRE(f.ac.getMode() == MODE_COOL && f.ac.getTargetTemp() == 24);
        REQUIRE(f.ac.getIndoorTemp() == 17 && f.ac.getOutdoorTemp() == 11);
      });
      REQUIRE(f.ac.m_readStatus(data) == RESPONSE_OK);
      REQUIRE(f.ac.hasFreshStatus() && climateUpdates == 1);
      f.ac.m_readStatus(data); REQUIRE(climateUpdates == 1);
      Control control; control.targetTemp = 26.5f; f.ac.control(control); f.ac.loop();
      REQUIRE(f.stream.tx.size() == 1 && f.stream.tx.back()[9] == DEVICE_CONTROL);
      const auto &tx = f.stream.tx.back();
      REQUIRE((tx[12] & 0x1F) == 0x1A && (tx[12] & 0xE0) == 0x40);
      // Ordinary climate controls intentionally retain their preset fan constraints.
      REQUIRE(tx[13] == (payloadSize == 24 ? FAN_AUTO : FAN_LOW));
      REQUIRE(FrameData(tx.data() + 10, tx.size() - 11).hasValidCRC());
      f.receive(data);
    }
  }
}

using Test = std::pair<const char *, void(*)()>;
int main(int argc, char **argv) {
  const Test tests[] = {
    {"seventeen_byte_payload_has_no_phantom_humidity", seventeen_byte_payload_has_no_phantom_humidity},
    {"climate_optional_fields_exclude_reply_integrity", climate_optional_fields_exclude_reply_integrity},
    {"nonstatus_uart_reply_preserves_guarded_retry", nonstatus_uart_reply_preserves_guarded_retry},
    {"truncated_c0_uart_reply_revokes_guarded_retry", truncated_c0_uart_reply_revokes_guarded_retry},
    {"purifier_toggle_preserves_independent_preset_flags", purifier_toggle_preserves_independent_preset_flags},
    {"purifier_command_does_not_echo_response_bits", purifier_command_does_not_echo_response_bits},
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
