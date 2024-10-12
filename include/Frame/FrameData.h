#pragma once
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace dudanov {
namespace midea {

/**
 * @brief Message body.
 *
 */
class FrameData {
  friend class Frame;

 public:
  /**
   * @brief Default constructor. Data vector is empty.
   */
  explicit FrameData() = default;

  /**
   * @brief Move this object to derivied frame data object.
   *
   * @tparam T frame data class type.
   * @return New data frame object.
   */
  template<typename T> T to() { return T(std::move(*this)); }

  /**
   * @brief Access for const element by index.
   *
   * @param idx index of element.
   * @return const uint8_t& element.
   */
  const uint8_t &operator[](int idx) const { return m_at(idx); }

  /**
   * @brief Access for element by index.
   *
   * @param idx index of element.
   * @return uint8_t& element.
   */
  uint8_t &operator[](int idx) { return m_at(idx); }

  /**
   * @brief Checking for data has specified `typeID`.
   *
   * @param typeID type ID.
   */
  bool hasID(uint8_t typeID) const { return m_data.front() == typeID; }

  bool hasPowerInfo() const { return this->hasID(0xC1); }

  /**
   * @brief Checks for properties in `FrameData`.
   *
   * @return `true` if `FrameData` has properties.
   */
  bool hasProperties(const FrameData &s) const { return this->hasID(0xB0) || this->hasID(0xB1) || this->hasID(0xB5); }

  /**
   * @brief Calculates and appends checksum.
   *
   */
  void appendCRC() { m_data.push_back(m_calcCRC()); }

  /**
   * @brief Updates the checksum.
   *
   */
  void updateCRC() {
    m_data.pop_back();
    this->appendCRC();
  }

  /**
   * @brief Appends variadic number of integer arguments.
   *
   * @param data first argument.
   * @param ...args other arguments.
   */
  template<typename T, typename... Args> void append(const T &data, Args... args) {
    this->append(data);
    this->append(args...);
  }

  /**
   * @brief Appends integer data.
   *
   * @tparam T integer data type.
   * @param data data to append.
   */
  template<typename T> void append(const T &data) {
    for (size_t n = 0; n != sizeof(T); ++n)
      m_data.push_back(data >> (n * 8));
  }

 protected:
  /**
   * @brief Data vector.
   *
   */
  std::vector<uint8_t> m_data;

  /**
   * @brief Init data with initializer list of bytes.
   *
   * @param lst list of bytes.
   */
  explicit FrameData(std::initializer_list<uint8_t> lst) : m_data{lst} {}

  /**
   * @brief Constructs data body from begin and end data iterators.
   *
   * @param begin begin iterator.
   * @param end end iterator.
   */
  FrameData(const uint8_t *begin, const uint8_t *end) : m_data{begin, end} {}

  /**
   * @brief Message ID generator.
   *
   * @return Message ID.
   */
  static uint8_t m_genID();

  /**
   * @brief Calculate checksum.
   *
   * @return Checksum.
   */
  uint8_t m_calcCRC() const;

  /**
   * @brief Access for const element by index.
   *
   * @param idx index of element.
   * @return const uint8_t& element.
   */
  const uint8_t &m_at(int idx) const { return (idx >= 0 ? m_data.begin() : m_data.end())[idx]; }

  /**
   * @brief Access for element by index.
   *
   * @param idx index of element.
   * @return uint8_t& element.
   */
  uint8_t &m_at(int idx) { return (idx >= 0 ? m_data.begin() : m_data.end())[idx]; }

  /**
   * @brief Get value from data body.
   *
   * @param idx byte index.
   * @param mask value mask (default: 0xFF).
   * @param rshift number of bits right shift (default: 0).
   * @return Value.
   */
  uint8_t m_getValue(int idx, uint8_t mask = 255, uint8_t rshift = 0) const { return (m_at(idx) >> rshift) & mask; }

  /**
   * @brief Get bit value from data body.
   *
   * @param idx byte index.
   * @param bit bit number.
   * @return Bit state.
   */
  bool m_getBit(int idx, uint8_t bit) const { return m_getValue(idx, 1, bit); }

  /**
   * @brief Set value to data body.
   *
   * @param idx byte index.
   * @param value value without shift.
   * @param mask value mask without bits shift (default: 0xFF). Used to clear destination bits.
   * @param lshift number of bits to left shift value and mask.
   */
  void m_setValue(int idx, uint8_t value, uint8_t mask = 255, uint8_t lshift = 0) {
    auto &x = m_at(idx);
    x &= ~(mask << lshift);
    x |= (value << lshift);
  }

  /**
   * @brief Set or clear bitmask.
   *
   * @param idx byte index.
   * @param state if `true` bitmask is set, else clear.
   * @param mask bitmask.
   */
  void m_setMask(int idx, bool state, uint8_t mask = 255) { m_setValue(idx, state ? mask : 0, mask); }

  /**
   * @brief Set bit state.
   *
   *
   * @param idx byte index.
   * @param bit bit number.
   * @param state bit state.
   */
  void m_setBit(int idx, uint8_t bit, bool state) { m_setMask(idx, state, 1 << bit); }
};

}  // namespace midea
}  // namespace dudanov
