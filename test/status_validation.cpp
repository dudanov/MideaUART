#include <cassert>
#include <cstdio>
#include <deque>
#include <sstream>
#include <tuple>
#include "Appliance/AirConditioner/AirConditioner.h"
#include "fixtures/status_validation.h"

using namespace dudanov::midea;
unsigned long test_millis = 0;

struct FakeStream : Stream {
  std::deque<uint8_t> rx;
  std::vector<std::vector<uint8_t>> tx;
  int available() override { return rx.size(); }
  int read() override { auto value = rx.front(); rx.pop_front(); return value; }
  int peek() override { return rx.front(); }
  size_t write(uint8_t value) override { return write(&value, 1); }
  size_t write(const uint8_t *data, size_t size) override {
    tx.emplace_back(data, data + size);
    return size;
  }
  void flush() override {}
  void inject(const std::vector<uint8_t> &frame) { rx.insert(rx.end(), frame.begin(), frame.end()); }
  void inject(const FrameData &payload, uint8_t type = DEVICE_QUERY) {
    Frame frame(0xAC, 3, type, payload);
    inject(std::vector<uint8_t>(frame.data(), frame.data() + frame.size()));
  }
};

struct TestAC : ac::AirConditioner {
  // Tests enqueue requests explicitly; disable background capability/power/status polling.
  void m_setup() override {}
  void m_onIdle() override {}
  void status(Handler success, Handler error) {
    m_queueRequest(DEVICE_QUERY, ac::QueryStateData{},
                  [this](FrameData data) { return m_readStatus(data); }, success, error);
  }
  ResponseStatus parse(FrameData data) { return m_readStatus(data); }
  std::vector<uint8_t> cached() const { return {m_status.data(), m_status.data() + m_status.size()}; }
  bool controlPending() const { return m_sendControl; }
  auto state() const {
    return std::make_tuple(getMode(), getTargetTemp(), getIndoorTemp(), getOutdoorTemp(),
                           getFanMode(), getSwingMode(), getPreset(), getIndoorHum());
  }
};

std::vector<uint8_t> bytes(const char *hex) {
  std::istringstream input(hex);
  unsigned int value;
  std::vector<uint8_t> result;
  while (input >> std::hex >> value) result.push_back(value);
  return result;
}

FrameData payload(const std::vector<uint8_t> &frame) {
  return FrameData(frame.data() + 10, frame.size() - 11);
}

bool valid_checksum(const FrameData &data) {
  unsigned int sum = 0;
  for (size_t i = 0; i < data.size(); ++i) sum += data.data()[i];
  return data.size() != 0 && (sum & 255) == 0;
}

struct Fixture {
  FakeStream stream;
  TestAC device;
  int publications = 0;
  int accepted = 0;
  int failed = 0;

  Fixture() {
    device.addOnStateCallback([this]() { ++publications; });
    device.setStream(&stream);
    device.setPeriod(1);
    device.setTimeout(100);
    device.setNumAttempts(3);
    device.setup();
    tick();
    tick();
    stream.tx.clear();
  }
  void tick(unsigned long ms = 1) { test_millis += ms; device.loop(); }
  void query() {
    device.status([this]() { ++accepted; }, [this]() { ++failed; });
    tick();
  }
  void seed() {
    query();
    stream.inject(bytes(STATUS_FIXTURES[0].hex));
    tick();
    assert(accepted == 1 && publications == 1 && device.getTargetTemp() == 23.5F);
  }
};

void test_captured_replies() {
  Fixture f;
  f.seed();
  for (const auto &fixture : STATUS_FIXTURES) {
    const auto frame = bytes(fixture.hex);
    uint8_t outer = 0;
    for (size_t i = 1; i < frame.size(); ++i) outer += frame[i];
    assert(outer == 0 && frame.size() == size_t(frame[1]) + 1);
    const auto state = f.device.state();
    const auto cached = f.device.cached();
    const auto publications = f.publications;
    const auto accepted = f.accepted;
    f.query();
    f.stream.inject(frame);
    f.tick();
    if (fixture.valid) {
      assert(f.accepted == accepted + 1 && f.failed == 0);
      assert(f.device.getMode() == ac::MODE_COOL && f.device.getTargetTemp() == 23.5F);
    } else {
      assert(!payload(frame).hasValidCRC() && !valid_checksum(payload(frame)));
      assert(f.accepted == accepted && f.failed == 0 && f.publications == publications);
      assert(f.device.state() == state && f.device.cached() == cached);
      f.stream.inject(bytes(STATUS_FIXTURES[0].hex));
      f.tick();
      assert(f.accepted == accepted + 1);  // Original request is still pending.
    }
  }
}

void test_payload_formats_and_lengths() {
  Fixture f;
  f.seed();
  const auto cached = f.device.cached();
  const auto state = f.device.state();
  const auto publications = f.publications;
  assert(f.device.parse(FrameData(uint8_t(0))) == RESPONSE_WRONG);
  for (uint8_t size = 1; size < 17; ++size) {
    std::vector<uint8_t> short_data(size - 1, 0);
    if (!short_data.empty()) short_data[0] = 0xC0;
    FrameData data(short_data.begin(), short_data.end());
    data.appendCRC();
    assert(data.hasValidCRC() && f.device.parse(data) == RESPONSE_WRONG);
  }
  assert(f.device.cached() == cached && f.device.state() == state && f.publications == publications);

  // Actual checksum-only C0 payload from https://github.com/mill1000/midea-ac-py/issues/11.
  const auto legacy = bytes("C0 00 4A 66 7F 7F 00 00 00 00 00 6B 67 00 00 00 00 00 00 C0");
  const FrameData legacy_body(legacy.data(), legacy.size());
  assert(!legacy_body.hasValidCRC() && valid_checksum(legacy_body));
  assert(f.device.parse(legacy_body) == RESPONSE_OK);
  assert(f.device.getMode() == ac::MODE_OFF && f.device.getTargetTemp() == 26);
  assert(f.device.getIndoorHum() == 0);  // Final 0xC0 is checksum, not 64% humidity.

  // Synthetic decoder boundaries, not additional model captures.
  const auto good = bytes(STATUS_FIXTURES[0].hex);
  for (size_t length : {16, 19, 21, 24, 40}) {
    std::vector<uint8_t> prefix(good.begin() + 10, good.begin() + 26);
    prefix.resize(length, 0);
    prefix[15] = 0;
    FrameData data(prefix.begin(), prefix.end());
    data.appendCRC();
    assert(f.device.parse(data) == RESPONSE_OK);
    assert(f.device.getTargetTemp() == 23.5F && f.device.getIndoorHum() == 0);
    assert(f.device.getPreset() == ac::PRESET_NONE);
  }
  auto wrong_id = payload(good);
  std::vector<uint8_t> other(wrong_id.data(), wrong_id.data() + wrong_id.size() - 1);
  other[0] = 0xC1;
  FrameData data(other.begin(), other.end());
  data.appendCRC();
  assert(data.hasValidCRC() && f.device.parse(data) == RESPONSE_WRONG);
}

void test_retry_deadline_and_exhaustion() {
  Fixture f;
  f.seed();
  const auto bad = bytes(STATUS_FIXTURES[1].hex);
  const auto good = bytes(STATUS_FIXTURES[0].hex);
  f.query();
  const auto sent = f.stream.tx.size();
  f.tick(90);
  f.stream.inject(bad);
  f.tick();
  assert(f.stream.tx.size() == sent && f.accepted == 1 && f.failed == 0);
  f.tick(10);
  assert(f.stream.tx.size() == sent + 1);  // Bad reply did not restart the deadline.
  f.stream.inject(good);
  f.tick();
  assert(f.accepted == 2 && f.failed == 0);

  f.query();
  const auto first_attempt = f.stream.tx.size();
  for (int attempt = 0; attempt < 3; ++attempt) {
    f.stream.inject(bad);
    f.tick();
    f.tick(100);
    assert(f.stream.tx.size() == first_attempt + (attempt < 2 ? attempt + 1 : 2));
    assert(f.accepted == 2 && f.failed == (attempt == 2 ? 1 : 0));
  }
  f.tick(100);
  assert(f.failed == 1 && f.stream.tx.size() == first_attempt + 2);
  f.query();
  f.stream.inject(good);
  f.tick();
  assert(f.accepted == 3);  // Exhaustion did not block the queue.
}

void test_control_and_display_replies() {
  Fixture f;
  f.seed();
  const auto bad = payload(bytes(STATUS_FIXTURES[9].hex));
  const auto good = payload(bytes(STATUS_FIXTURES[0].hex));
  ac::Control command;
  command.targetTemp = 24.0F;
  f.device.control(command);
  f.tick();
  assert(f.device.controlPending());
  const auto cached = f.device.cached();
  const auto state = f.device.state();
  const auto publications = f.publications;
  f.stream.inject(bad, DEVICE_CONTROL);
  f.tick();
  assert(f.device.controlPending() && f.device.cached() == cached && f.device.state() == state);
  assert(f.publications == publications);
  f.stream.inject(good, DEVICE_CONTROL);
  f.tick();
  assert(!f.device.controlPending());

  f.device.displayToggle();
  f.tick();
  const auto sent = f.stream.tx.size();
  f.stream.inject(bad);
  f.tick();
  assert(f.publications == publications && f.device.cached() == cached && f.device.state() == state);
  f.tick(100);
  assert(f.stream.tx.size() == sent + 1);  // Display request also remains pending.
  f.stream.inject(good);
  f.tick();
  f.tick(100);
  assert(f.stream.tx.size() == sent + 1);
}

int main() {
  test_captured_replies();
  test_payload_formats_and_lengths();
  test_retry_deadline_and_exhaustion();
  test_control_and_display_replies();
  puts("PASS: status validation, payload formats, retries, control and display replies");
}
