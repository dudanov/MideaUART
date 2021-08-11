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
  : m_data({0xAA, 0x00, appliance, 0x00, 0x00, 0x00, 0x00, 0x00, protocol, type}) {
    this->setData(data);
  }
  FrameData getData() const { return FrameData(this->m_data.data() + OFFSET_DATA, this->m_len() - OFFSET_DATA); }
  void setData(const FrameData &data) {
    this->m_trimData();
    this->m_appendData(data);
    this->m_data[OFFSET_LENGTH] = this->m_data.size();
    this->m_data[OFFSET_SYNC] = this->m_data[OFFSET_LENGTH] ^ this->m_data[OFFSET_APPTYPE];
    this->appendCS();
  }
  const uint8_t *data() const { return this->m_data.data(); }
  uint8_t size() const { return this->m_data.size(); }
  void setType(uint8_t value) { this->m_data[9] = value; }
  bool hasType(uint8_t value) const { return this->m_data[9] == value; }
  uint8_t getType() const { return this->m_data[9]; }

  bool hasValidCS() const { return !this->m_calcCS(); }
  void appendCS() { this->m_data.push_back(this->m_calcCS()); }

  void setProtocol(uint8_t value) { this->m_data[8] = value; }
  uint8_t getProtocol() const { return this->m_data[8]; }

  bool hasApp(uint8_t value) const { return this->m_data[OFFSET_APPTYPE] == value; }
  uint8_t appType() const { return this->m_data[OFFSET_APPTYPE]; }

 protected:
  static const uint8_t START_BYTE = 0xAA;
  static const uint8_t OFFSET_START = 0;
  static const uint8_t OFFSET_LENGTH = 1;
  static const uint8_t OFFSET_APPTYPE = 2;
  static const uint8_t OFFSET_SYNC = 3;
  static const uint8_t OFFSET_DATA = 10;
  void m_trimData() { this->m_data.erase(this->m_data.begin() + OFFSET_DATA, this->m_data.end()); }
  void m_appendData(const FrameData &data) { std::copy(data.data(), data.data() + data.size(), std::back_inserter(this->m_data)); }
  uint8_t m_len() const { return this->m_data[OFFSET_LENGTH]; }
  void m_updateLen() { this->m_data[OFFSET_LENGTH] = this->m_data.size(); }
  uint8_t m_calcCS() const;
  std::vector<uint8_t> m_data;
};

}  // namespace midea
}  // namespace dudanov
