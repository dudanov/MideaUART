#include <cassert>
#include <cstdio>
#include <deque>
#include <sstream>
#include <tuple>
#include "Appliance/AirConditioner/AirConditioner.h"
#include <cmath>

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
  void m_setup() override {}
  void m_onIdle() override {}
  void normal(Handler error) {
    m_queueRequest(DEVICE_QUERY, FrameData({0x41}), [](FrameData) { return RESPONSE_WRONG; }, nullptr, error);
  }
  void power() { m_getPowerUsage(); }
  void priority(Handler success) {
    m_queueRequestPriority(DEVICE_CONTROL, FrameData({0x40}),
      [](FrameData data) { return data.hasID(0xC0) ? RESPONSE_OK : RESPONSE_WRONG; }, success);
  }
};
struct Fixture {
  FakeStream stream;
  TestAC device;
  int accepted = 0, failed = 0;
  float watts = NAN;
  Fixture() {
    device.setStream(&stream);
    device.setPeriod(1);
    device.setTimeout(100);
    device.setNumAttempts(3);
    device.setup();
    tick(); tick(); stream.tx.clear();
  }
  void tick(unsigned long ms = 1) { test_millis += ms; device.loop(); }
  bool query() {
    return device.requestGroup7Power([this](float value) { watts = value; ++accepted; },
                                    [this]() { ++failed; });
  }
};
FrameData group7(unsigned int watts) {
  FrameData data({0xC1,0x21,0x01,0x47,0,0,0,0,0,0,uint8_t(watts),uint8_t(watts >> 8)});
  data.appendCRC();
  return data;
}
FrameData group4(uint8_t hundreds) {
  FrameData data({0xC1,0x21,0x01,0x44,0,0,0,0,0,0,0,0,0,0,0,0,0,hundreds,0});
  data.appendCRC();
  return data;
}
void test_one_attempt_and_recovery() {
  Fixture f;
  assert(f.query()); assert(!f.query()); assert(f.stream.tx.empty());
  f.tick();
  assert(f.stream.tx.size() == 1);
  const auto &tx = f.stream.tx[0];
  const std::vector<uint8_t> expected{0x41,0x21,0x01,0x47,0,1,0xED};
  assert(std::equal(expected.begin(), expected.end(), tx.begin()+10));
  f.tick(101); f.tick(1000);
  assert(f.failed == 1 && f.accepted == 0 && f.stream.tx.size() == 1);
  f.stream.inject(group7(123)); f.tick();  // Unsolicited late reply is not published.
  assert(f.accepted == 0);
  assert(f.query()); f.tick(); f.stream.inject(group7(269)); f.tick();
  assert(f.accepted == 1 && f.watts == 269 && f.failed == 1);
  int failed = 0;
  f.device.normal([&]() { ++failed; });
  f.tick(); f.tick(101); f.tick(101); f.tick(101);
  assert(failed == 1 && f.stream.tx.size() == 5);  // Ordinary requests retain three attempts.
}
void test_payload_validation() {
  Fixture f;
  for (size_t length = 0; length < 13; ++length) {
    assert(f.query()); f.tick();
    auto valid = group7(269);
    FrameData short_reply(valid.data(), length);
    if (length > 0) {  // Also exercise valid-CRC truncations.
      short_reply = FrameData(valid.data(), length-1);
      short_reply.appendCRC();
    }
    f.stream.inject(short_reply); f.tick(); assert(f.accepted == 0);
    f.tick(101);
  }
  assert(f.failed == 13);
  for (size_t index : {size_t(0), size_t(1), size_t(2), size_t(3)}) {
    auto valid = group7(269);
    std::vector<uint8_t> body(valid.data(), valid.data()+12);
    body[index] ^= 1;
    FrameData wrong(body.data(), body.size()); wrong.appendCRC();
    assert(f.query()); f.tick(); f.stream.inject(wrong); f.tick(); assert(f.accepted == 0);
    f.tick(101);
  }
  auto valid = group7(269);
  std::vector<uint8_t> corrupt(valid.data(),valid.data()+valid.size()); corrupt.back() ^= 1;
  assert(f.query()); f.tick(); f.stream.inject(FrameData(corrupt.data(),corrupt.size())); f.tick();
  assert(f.accepted == 0); f.tick(101);
  for (auto value : {0U, 269U, 65535U}) {
    assert(f.query()); f.tick(); f.stream.inject(group7(value)); f.tick();
    assert(f.watts == value);
  }
  // Actual Carrier capture: known inner CRC, extended payload, 739 W.
  FrameData captured({0xC1,0x21,0x01,0x47,0xD6,0xEB,0x4E,1,1,0,0xE3,2,0,0,0,0,0,0,0,0,1,0xE2});
  assert(captured.hasValidCRC());
  assert(f.query()); f.tick(); f.stream.inject(captured); f.tick();
  assert(f.watts == 739 && f.accepted == 4);
}
void test_group4_isolation() {
  Fixture f;
  f.device.power(); f.tick(); f.stream.inject(group4(0x50)); f.tick();
  assert(f.device.getPowerUsage() == 500);
  f.device.power(); f.tick(); f.stream.inject(group7(739)); f.tick();
  assert(f.device.getPowerUsage() == 500);
  f.stream.inject(group4(0x60)); f.tick();
  assert(f.device.getPowerUsage() == 600);
  assert(f.query()); f.tick(); f.stream.inject(group4(0x70)); f.tick();
  assert(f.accepted == 0); f.stream.inject(group7(269)); f.tick(); assert(f.accepted == 1);
  for (size_t length=0; length<20; ++length) {
    auto valid = group4(0x50);
    FrameData truncated(valid.data(),length);
    assert(!truncated.hasPowerInfo());
    assert(std::isnan(truncated.to<ac::StatusData>().getPowerUsage()));
  }
}
void test_queue_and_callbacks() {
  Fixture f;
  int controls=0;
  assert(f.query());
  f.device.priority([&]() { ++controls; });
  f.tick(); assert(f.stream.tx.back()[9] == DEVICE_CONTROL);
  f.stream.inject(FrameData({0xC0}), DEVICE_CONTROL); f.tick();
  assert(controls == 1); f.tick();
  assert(f.stream.tx.back()[13] == 0x47);
  f.stream.inject(group7(0)); f.tick(); assert(f.accepted == 1);
  // Consumer callbacks may enqueue another query after completion.
  assert(f.device.requestGroup7Power([&](float) { assert(f.query()); }));
  f.tick(); f.stream.inject(group7(1)); f.tick(); f.tick();
  f.stream.inject(group7(2)); f.tick(); assert(f.accepted == 2 && f.watts == 2);
  assert(f.device.requestGroup7Power(nullptr, [&]() { assert(f.query()); }));
  f.tick(); f.tick(101); f.tick(); f.stream.inject(group7(3)); f.tick();
  assert(f.accepted == 3 && f.watts == 3);
}
int main() {
  test_one_attempt_and_recovery();
  test_payload_validation();
  test_group4_isolation();
  test_queue_and_callbacks();
  puts("PASS: Group 7 queue, timeout/recovery, validation, decoding, Group 4 isolation and priority/callback tests");
}
