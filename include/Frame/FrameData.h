#pragma once
#include <Arduino.h>
#include <vector>

class IPAddress;

namespace dudanov {
namespace midea {

/**
 * @brief Message body.
 *
 */
class FrameData {
 public:
  FrameData() = default;

  FrameData(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
      : m_data(begin, end) {}

  FrameData(const uint8_t *data, uint8_t size) : m_data(data, data + size) {}

  FrameData(std::initializer_list<uint8_t> list) : m_data(list) {}

  FrameData(uint8_t size) : m_data(size, 0) {}

  template<typename T> T to() { return std::move(*this); }

  const uint8_t &operator[](size_t idx) const { return this->m_data[idx]; }

  uint8_t &operator[](size_t idx) { return this->m_data[idx]; }

  const uint8_t *data() const { return this->m_data.data(); }

  uint8_t size() const { return this->m_data.size(); }

  bool hasID(uint8_t value) const { return this->m_data[0] == value; }

  bool hasStatus() const { return this->hasID(0xC0); }

  bool hasPowerInfo() const { return this->hasID(0xC1); }

  /**
   * @brief Calculates and adds a checksum.
   *
   */
  void appendCRC() { this->m_data.push_back(this->m_calcCRC()); }

  /**
   * @brief Updates the checksum (last byte).
   *
   */
  void updateCRC() {
    this->m_data.pop_back();
    this->appendCRC();
  }

  /**
   * @brief Verifies the checksum.
   *
   * @return `true` if checksum is correct.
   */
  bool hasValidCRC() const { return this->m_calcCRC() == 0; }

  /**
   * @brief Appends data.
   *
   * @tparam T integer data type.
   * @param data data to append.
   */
  template<typename T> void append(const T &data) {
    for (size_t i = 0; i < sizeof(T); ++i)
      this->m_data.push_back(data >> (i * 8));
  }

 protected:
  /**
   * @brief Data vector.
   *
   */
  std::vector<uint8_t> m_data;

  /**
   * @brief Static function for generate message ID from a cyclic sequence from 4 to 254.
   *
   * @return ID from 4 to 254.
   */
  static uint8_t m_getID();

  /**
   * @brief Calculate checksum.
   *
   * @return Checksum.
   */
  uint8_t m_calcCRC() const;

  /**
   * @brief Get value from data body.
   *
   * @param idx byte index.
   * @param mask value mask (default: 0xFF).
   * @param rshift number of bits right shift (default: 0).
   * @return value.
   */
  uint8_t m_getValue(uint8_t idx, uint8_t mask = 255, uint8_t rshift = 0) const {
    return (this->m_data[idx] >> rshift) & mask;
  }

  /**
   * @brief Set value to data body.
   *
   * @param idx byte index.
   * @param value value without shift.
   * @param mask value mask without bits shift (default: 0xFF). Used to clear destination bits.
   * @param lshift number of bits to left shift value and mask.
   */
  void m_setValue(uint8_t idx, uint8_t value, uint8_t mask = 255, uint8_t lshift = 0) {
    this->m_data[idx] &= ~(mask << lshift);
    this->m_data[idx] |= (value << lshift);
  }

  /**
   * @brief Set or clear bitmask.
   *
   * @param idx byte index.
   * @param state if `true` bitmask is set, else clear.
   * @param mask bitmask.
   */
  void m_setMask(uint8_t idx, bool state, uint8_t mask = 255) { this->m_setValue(idx, state ? mask : 0, mask); }
};

/**
 * @brief FrameData for new commands 0xB0 (set), 0xB1 (get).
 *
 */
class NewFrameData : public FrameData {
 public:
  /**
   * @brief Initializes a new frame with ID.
   * @param id frame ID.
   */
  NewFrameData(uint8_t id);

  /**
   * @brief Append new command.
   *
   * @param uuid command 16-bit UUID.
   */
  void appendCommand(uint16_t uuid);

  /**
   * @brief Append command data.
   *
   * @param data command data.
   */
  void appendData(uint8_t data);

 protected:
  uint8_t m_lenIdx;
};

class NetworkNotifyData : public FrameData {
 public:
  NetworkNotifyData()
      : FrameData({0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

  void setConnected(bool state) { this->m_setMask(8, !state, 1); }

  void setSignalStrength(uint8_t value) { this->m_setValue(2, value); }

  void setIP(const IPAddress &ip);
};

}  // namespace midea
}  // namespace dudanov
