#pragma once
#include <Arduino.h>
#include <vector>
#include "Frame/FrameData.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {

class Frame {
 public:
  Frame() = default;
  Frame(uint8_t appliance, uint8_t protocol, uint8_t type, const FrameData &data)
  : m_data({START_BYTE, 0x00, appliance, 0x00, 0x00, 0x00, 0x00, 0x00, protocol, type}) {
    this->setData(data);
  }
  FrameData getData() const { return FrameData(this->m_data.data() + OFFSET_DATA, this->m_len() - OFFSET_DATA); }
  void setData(const FrameData &data);
  bool isValid() const { return !this->m_calcCS(); }

  const uint8_t *data() const { return this->m_data.data(); }
  uint8_t size() const { return this->m_data.size(); }
  void setType(uint8_t value) { this->m_data[OFFSET_TYPE] = value; }
  bool hasType(uint8_t value) const { return this->m_data[OFFSET_TYPE] == value; }
  void setProtocol(uint8_t value) { this->m_data[OFFSET_PROTOCOL] = value; }
  uint8_t getProtocol() const { return this->m_data[OFFSET_PROTOCOL]; }
  String toString() const;

 protected:
  std::vector<uint8_t> m_data;
  void m_trimData() { this->m_data.erase(this->m_data.begin() + OFFSET_DATA, this->m_data.end()); }
  void m_appendData(const FrameData &data) { std::copy(data.data(), data.data() + data.size(), std::back_inserter(this->m_data)); }
  uint8_t m_len() const { return this->m_data[OFFSET_LENGTH]; }
  void m_appendCS() { this->m_data.push_back(this->m_calcCS()); }
  uint8_t m_calcCS() const;
  static const uint8_t START_BYTE = 0xAA;
  static const uint8_t OFFSET_START = 0;
  static const uint8_t OFFSET_LENGTH = 1;
  static const uint8_t OFFSET_APPTYPE = 2;
  static const uint8_t OFFSET_SYNC = 3;
  static const uint8_t OFFSET_PROTOCOL = 8;
  static const uint8_t OFFSET_TYPE = 9;
  static const uint8_t OFFSET_DATA = 10;
};

}  // namespace midea
}  // namespace dudanov
