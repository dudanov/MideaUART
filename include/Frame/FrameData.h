#pragma once
#include <Arduino.h>
#include <vector>

class IPAddress;

namespace dudanov {
namespace midea {

class FrameData {
 public:
  FrameData() = delete;
  FrameData(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end) : m_data(begin, end) {}
  FrameData(const uint8_t *data, uint8_t size) : m_data(data, data + size) {}
  FrameData(std::initializer_list<uint8_t> list) : m_data(list) {}
  FrameData(uint8_t size) : m_data(size, 0) {}
  template<typename T> T to() { return std::move(*this); }
  const uint8_t *data() const { return this->m_data.data(); }
  uint8_t size() const { return this->m_data.size(); }
  bool hasID(uint8_t value) const { return this->m_data[0] == value; }
  bool hasStatus() const { return this->hasID(0xC0); }
  bool hasPowerInfo() const { return this->hasID(0xC1); }
  void appendCRC() { this->m_data.push_back(this->m_calcCRC()); }
  void updateCRC() {
    this->m_data.pop_back();
    this->appendCRC();
  }
  bool hasValidCRC() const { return !this->m_calcCRC(); }
 protected:
  std::vector<uint8_t> m_data;
  static uint8_t m_id;
  static uint8_t m_getID() { return FrameData::m_id++; }
  static uint8_t m_getRandom() { return random(255) + 1; }
  uint8_t m_calcCRC() const;
  uint8_t m_getValue(uint8_t idx, uint8_t mask = 255, uint8_t shift = 0) const {
    return (this->m_data[idx] >> shift) & mask;
  }
  void m_setValue(uint8_t idx, uint8_t value, uint8_t mask = 255, uint8_t shift = 0) {
    this->m_data[idx] &= ~(mask << shift);
    this->m_data[idx] |= (value << shift);
  }
  void m_setMask(uint8_t idx, bool state, uint8_t mask = 255) { this->m_setValue(idx, state ? mask : 0, mask); }
};

class NetworkNotifyData : public FrameData {
 public:
  NetworkNotifyData() : FrameData({0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}
  void setConnected(bool state) { this->m_setMask(8, !state, 1); }
  void setSignalStrength(uint8_t value) { this->m_setValue(2, value); }
  void setIP(const IPAddress &ip);
};

}  // namespace midea
}  // namespace dudanov
