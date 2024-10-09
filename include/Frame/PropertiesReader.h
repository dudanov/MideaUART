#pragma once
#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {

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
   * @brief Size of properties data.
   *
   * @return size of properties data.
   */
  int size() const { return m_pdata[-1]; }

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
   * @brief Checks whether the current property has valid, readable header.
   *
   */
  bool hasHeader() const { return m_available() >= 0; }

  /**
   * @brief Checks whether the current property has valid, readable data.
   *
   */
  bool hasData() const { return m_available() > 0 && this->size() > 0 && m_available() >= this->size(); }

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
  // Pointer to begin of current property.
  const uint8_t *m_begin() const { return m_pheader; }

  // Pointer to end of current property (begin of next property).
  const uint8_t *m_end() const { return m_pdata + this->size(); }

  // Size of current property.
  int m_size() const { return std::distance(m_begin(), m_end()); }

  // Available data bytes for read. Always must be greater or equals to `size()`.
  int m_available() const { return std::distance(m_pdata, m_pend); }

  const uint8_t *m_pheader;     // pointer to header
  const uint8_t *m_pdata;       // pointer to data
  const uint8_t *const m_pend;  // pointer to data end
};

}  // namespace midea
}  // namespace dudanov
