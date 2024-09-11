#pragma once

#include "FrameData.h"

namespace dudanov {
namespace midea {

/**
 * @brief Type of property's 16-bit UUID.
 *
 */
using PropertyUUID = uint16_t;

/**
 * @brief Properties data reader.
 *
 */
class PropertiesReader {
 public:
  /**
   * @brief Constructor from `PropertiesData`. Skip `ID`, `NUM` and `CRC` fields.
   *
   * @param s reference to `FrameData`.
   */
  explicit PropertiesReader(const FrameData &s);

  PropertiesReader(const PropertiesReader &) = delete;

  /**
   * @brief Checks for properties in `FrameData`.
   *
   * @param s reference to `FrameData`.
   * @return `true` if `FrameData` has properties.
   */
  static bool hasProperties(const FrameData &s);

  /**
   * @brief Size of properties data.
   *
   * @return size of properties data.
   */
  uint8_t size() const { return m_pdata[-1]; }

  /**
   * @brief Result of operation. Valid only for `0xB0` and `0xB1` messages.
   *
   * @return Result of operation.
   */
  uint8_t result() const { return m_pdata[-2]; }

  /**
   * @brief Property data access `operator[]`.
   *
   * @param idx byte index.
   * @return property byte.
   */
  const uint8_t &operator[](int idx) const { return m_pdata[idx]; }

  /**
   * @brief Available bytes for read.
   *
   * @return int Available bytes for read.
   */
  int available() const { return std::distance(m_pdata + this->size(), m_pend); }

  /**
   * @brief Current property is valid.
   *
   * @return property validation result.
   */
  bool valid() const { return this->available() >= 0; }

  /**
   * @brief Property UUID.
   *
   * @return UUID.
   */
  PropertyUUID uuid() const { return m_pheader[1] * 256 + m_pheader[0]; }

  /**
   * @brief Advance reader to next property.
   *
   */
  void advance();

 private:
  size_t m_hdrLen() const { return std::distance(m_pheader, m_pdata); }

  const uint8_t *m_pheader;     // pointer to header
  const uint8_t *m_pdata;       // pointer to data
  const uint8_t *const m_pend;  // pointer to data end
};

/**
 * @brief FrameData for new commands 0xB0 (set), 0xB1 (get).
 *
 */
class PropertiesData : public FrameData {
 public:
  /**
   * @brief Makes `PropertiesData` instance of specified type ID.
   *
   * @param id type ID.
   * @return `PropertiesData` instance.
   */
  static PropertiesData fromID(uint8_t id) { return PropertiesData{id}; }

  /**
   * @brief Append `appendUUID` command (used in 0xB1 GET queries).
   *
   * @param uuid UUID of property.
   */
  void appendUUID(PropertyUUID uuid);

  /**
   * @brief Adds a Pascal-type byte array. The array size is added automatically.
   *
   * @param ...data bytes to set.
   */
  template<typename... Args> void appendPascalArray(Args... data) {
    this->append(static_cast<uint8_t>(sizeof...(Args)), static_cast<uint8_t>(data)...);
  }

  /**
   * @brief Append property with specified byte array.
   *
   * @param uuid Property UUID.
   * @param data bytes to set.
   */
  template<typename... Args> void appendProperty(PropertyUUID uuid, Args... data) {
    this->appendUUID(uuid);
    this->appendPascalArray(data...);
  }

 protected:
  explicit PropertiesData(uint8_t id) : FrameData{{id, 0}} {}
};

}  // namespace midea
}  // namespace dudanov
