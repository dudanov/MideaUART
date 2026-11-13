#include <cassert>
#include <cstdio>
#include <deque>
#include <fstream>
#include <random>
#include <vector>
#include "Appliance/ApplianceBase.h"

using namespace dudanov;
using namespace dudanov::midea;
using Bytes = std::vector<uint8_t>;

class Wire : public Stream {
 public:
  std::deque<uint8_t> input;
  std::vector<Bytes> writes;
  bool negative_read{};
  int available() override { return negative_read ? 1 : input.size(); }
  int read() override {
    if (negative_read || input.empty()) return -1;
    const auto byte = input.front();
    input.pop_front();
    return byte;
  }
  int peek() override { return input.empty() ? -1 : input.front(); }
  size_t write(uint8_t byte) override { return write(&byte, 1); }
  size_t write(const uint8_t *data, size_t size) override {
    writes.emplace_back(data, data + size);
    return size;
  }
  void flush() override { assert(false && "Recovery must not flush UART input"); }
  void inject(const Bytes &bytes) { input.insert(input.end(), bytes.begin(), bytes.end()); }
};

class Probe : public ApplianceBase {
 public:
  std::vector<Bytes> frames;
  explicit Probe(ApplianceType type = AIR_CONDITIONER) : ApplianceBase(type) {}
  void m_onRequest(const Frame &frame) override {
    // Frame::size() is uint8_t; derive the full 256-byte boundary from length.
    Bytes bytes(frame.data(), frame.data() + size_t(frame.data()[1]) + 1);
    uint8_t sum = 0;
    for (size_t i = 1; i < bytes.size(); ++i) sum += bytes[i];
    assert(sum == 0);
    frames.push_back(bytes);
  }
};

struct Rig {
  Wire wire;
  Probe parser;
  explicit Rig(ApplianceType type = AIR_CONDITIONER) : parser(type) {
    test_clock_us = 1000000;
    parser.setStream(&wire);
  }
  void send(const Bytes &bytes) { wire.inject(bytes); parser.loop(); }
  void tick(uint32_t ms) { test_clock_us += int64_t(ms) * 1000; parser.loop(); }
};

Bytes load(const char *path) {
  std::ifstream file(path);
  assert(file);
  Bytes bytes;
  unsigned value;
  while (file >> std::hex >> value) bytes.push_back(value);
  return bytes;
}

Bytes join(Bytes a, const Bytes &b) {
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

Bytes frame(const Bytes &body, uint8_t appliance = AIR_CONDITIONER) {
  Frame value(appliance, 2, DEVICE_QUERY, FrameData(body.data(), body.size()));
  return Bytes(value.data(), value.data() + size_t(value.data()[1]) + 1);
}

class QueryProbe : public ApplianceBase {
 public:
  unsigned successes{}, failures{};
  QueryProbe() : ApplianceBase(AIR_CONDITIONER) {}
  void request(const Bytes &expected) {
    // Exercise existing request dispatch with an explicit CRC-checking handler.
    // Do not change the upstream appliance handlers as part of parser recovery.
    m_queueRequest(DEVICE_QUERY, FrameData({0x41, 0x81, 0x00, 0xFF}),
      [expected](FrameData body) {
        return body.hasValidCRC() && Bytes(body.data(), body.data() + body.size()) == expected
            ? RESPONSE_OK : RESPONSE_WRONG;
      }, [this] { ++successes; }, [this] { ++failures; });
  }
};

struct QueryRig {
  Wire wire;
  QueryProbe parser;
  explicit QueryRig(const Bytes &expected) {
    test_clock_us = 1000000;
    TimerManager clock;
    clock.task(); // Synchronize the shared timer clock before setup starts timers.
    parser.setStream(&wire);
    parser.setup();
    parser.loop(); // Send the setup network notification.
    tick(1001);   // Complete its period timer.
    parser.request(expected);
    parser.loop();
    wire.writes.clear();
  }
  void tick(uint32_t ms) { test_clock_us += int64_t(ms) * 1000; parser.loop(); }
};

int main(int argc, char **argv) {
  assert(argc == 3);
  const auto truncated = load(argv[1]), false_header = load(argv[2]);
  assert(truncated.size() == 80 && false_header.size() == 96);
  const Bytes good(truncated.begin() + 39, truncated.end());
  const Bytes group(false_header.begin() + 30, false_header.begin() + 63);
  assert(Bytes(false_header.begin() + 63, false_header.end()) == group);
  assert(FrameData(good.data() + 10, good.size() - 11).hasValidCRC());
  assert(FrameData(group.data() + 10, group.size() - 11).hasValidCRC());

  for (size_t split = 0; split <= truncated.size(); ++split) {
    Rig r;
    r.send(Bytes(truncated.begin(), truncated.begin() + split));
    r.tick(10);
    r.send(Bytes(truncated.begin() + split, truncated.end()));
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  for (size_t split = 0; split <= false_header.size(); ++split) {
    Rig r;
    r.send(Bytes(false_header.begin(), false_header.begin() + split));
    r.tick(10);
    r.send(Bytes(false_header.begin() + split, false_header.end()));
    assert(r.parser.frames == std::vector<Bytes>({group, group}));
  }
  {
    Rig r;
    for (auto byte : false_header) { r.send({byte}); r.tick(1); }
    assert(r.parser.frames == std::vector<Bytes>({group, group}));
  }
  {
    Rig r(DEHUMIDIFIER);
    const auto other = frame({0x11, 0x22}, DEHUMIDIFIER);
    r.send(join(good, other));
    assert(r.parser.frames == std::vector<Bytes>{other});
  }
  {
    Rig r;
    for (auto byte : good) { r.send({byte}); r.tick(19); }
    assert(r.parser.frames == std::vector<Bytes>{good}); // Total time > 500 ms.
  }
  {
    Rig r;
    r.send(join(join(good, good), good));
    assert(r.parser.frames == std::vector<Bytes>({good, good, good}));
  }
  {
    const auto outer = frame(join({0xC0, 0xAA}, good));
    Rig r;
    r.send(Bytes(outer.begin(), outer.end() - 1));
    r.tick(499);
    assert(r.parser.frames.empty());
    r.send({outer.back()});
    assert(r.parser.frames == std::vector<Bytes>{outer});
  }
  for (unsigned length = 0; length <= 10; ++length) {
    Rig r;
    r.send(join({0xAA, uint8_t(length)}, good));
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    Rig r;
    r.send(join(join({0xAA, 0xF0, 0xAC, 0}, good), good));
    r.tick(499);
    assert(r.parser.frames.empty());
    r.tick(1);
    assert(r.parser.frames == std::vector<Bytes>({good, good}));
    r.tick(1000);
    assert(r.parser.frames.size() == 2);
  }
  {
    Rig r;
    r.send(join({0xAA, 0xF0, 0xAC, 0xAA, 0xEF, 0xAC}, good));
    r.tick(500);
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    auto outer = frame(join(join({0x11}, good), good));
    outer.back() ^= 1;
    Rig r;
    r.send(outer);
    assert(r.parser.frames == std::vector<Bytes>({good, good}));
  }
  {
    Rig r;
    r.send(Bytes(good.begin(), good.begin() + 15));
    test_clock_us += 900000; // Continuation is already queued after a slow loop.
    r.send(Bytes(good.begin() + 15, good.end()));
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    Rig r;
    r.send({0xAA});
    r.tick(499);
    r.send(Bytes(good.begin() + 1, good.end()));
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    Rig r;
    r.send({0xAA});
    r.tick(500);
    r.send(Bytes(good.begin() + 1, good.end()));
    assert(r.parser.frames.empty()); // The stale marker expired.
    r.send(good);
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    Rig r;
    test_clock_us = int64_t(UINT32_MAX - 200) * 1000;
    r.send(join({0xAA, 0xF0, 0xAC}, good));
    r.tick(499);
    assert(r.parser.frames.empty());
    r.tick(1);
    assert(r.parser.frames == std::vector<Bytes>{good});
  }
  {
    const auto largest = frame(Bytes(245, 0x11));
    assert(largest.size() == 256);
    Rig r;
    r.send(join(largest, good));
    assert(r.parser.frames == std::vector<Bytes>({largest, good}));
    auto bad = largest;
    bad.back() ^= 1;
    r.send(join(bad, good));
    assert(r.parser.frames.size() == 3 && r.parser.frames.back() == good);
  }
  {
    Rig r;
    r.send(Bytes(good.begin(), good.begin() + 15));
    r.wire.negative_read = true;
    r.tick(1000); // Unreadable available() must defer expiry.
    r.wire.negative_read = false;
    r.send(Bytes(good.begin() + 15, good.end()));
    assert(r.parser.frames == std::vector<Bytes>{good});
  }

  const Bytes status_body(good.begin() + 10, good.end() - 1);
  {
    QueryRig r(status_body);
    r.wire.inject(truncated);
    r.tick(50);
    assert(r.parser.successes == 1 && r.parser.failures == 0 && r.wire.writes.empty());
  }
  {
    QueryRig r(status_body);
    r.wire.inject(Bytes(truncated.begin(), truncated.begin() + 39));
    r.tick(50);
    r.tick(500);
    r.tick(1450); // The unchanged 2 s request timeout sends one retry.
    assert(r.wire.writes.size() == 1);
    r.wire.inject(good);
    r.tick(10);
    assert(r.parser.successes == 1 && r.parser.failures == 0);
  }
  {
    // Captured group bytes, modeled timing; a test handler checks exact body/CRC.
    QueryRig r(Bytes(group.begin() + 10, group.end() - 1));
    r.wire.inject(Bytes(false_header.begin(), false_header.begin() + 30));
    r.tick(50);
    r.wire.inject(group);
    r.tick(1950);
    assert(r.wire.writes.size() == 1 && r.parser.successes == 1 && r.parser.failures == 0);
  }
  {
    QueryRig r(status_body);
    r.wire.inject(join({0xAA, 0xF0, 0xAC}, good));
    r.tick(50);
    assert(r.parser.successes == 0);
    r.tick(500);
    assert(r.parser.successes == 1 && r.parser.failures == 0 && r.wire.writes.empty());
  }
  {
    // An outer-valid but inner-invalid reply is still rejected by the handler.
    Bytes bad_body = status_body;
    bad_body.back() ^= 1;
    QueryRig r(status_body);
    r.wire.inject(frame(bad_body));
    r.tick(50);
    assert(r.parser.successes == 0);
    r.wire.inject(good);
    r.tick(50);
    assert(r.parser.successes == 1 && r.parser.failures == 0);
  }
  {
    QueryRig r(status_body);
    r.tick(2000); r.tick(2000); r.tick(2000);
    assert(r.parser.failures == 1 && r.parser.successes == 0 && r.wire.writes.size() == 2);
  }
  {
    std::mt19937 random(0x56AC);
    Rig r;
    for (unsigned batch = 0; batch < 1000; ++batch) {
      Bytes noise(1000);
      for (auto &byte : noise) byte = random();
      r.send(noise);
      r.tick(10000);
      r.send(Bytes(256, 0));
      const auto count = r.parser.frames.size();
      r.send(good);
      assert(r.parser.frames.size() == count + 1 && r.parser.frames.back() == good);
      r.parser.frames.clear();
    }
  }
  puts("PASS: field replays, framing recovery, idle boundaries, CRC dispatch, retries, wrap, buffer bounds and 1M-byte stress");
}
