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
 * @brief FrameData for new commands 0xB0 (set), 0xB1 (get).
 *
 */
class PropertiesData : public FrameData {
 public:
  /**
   * @brief Makes `PropertiesData` instance from `FrameData` using `move semantics`.
   *
   * @param data source frame data.
   * @return `PropertiesData` instance.
   */
  static PropertiesData fromData(FrameData &data) { return PropertiesData{std::move(data)}; }

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

  /**
   * @brief Properties data reader.
   *
   */
  class PropertiesReader {
   public:
    /**
     * @brief Constructor from `PropertiesData`. Skip `ID`, `NUM` and `CRC` fields.
     *
     * @param src reference to `PropertiesData`.
     * @param headerLength header length.
     */
    explicit PropertiesReader(const PropertiesData &src, size_t headerLength)
        : m_header{&src.m_data[2]}, m_data{m_header + headerLength}, m_end{&src.m_data.back()} {}

    /**
     * @brief
     *
     *
     * @return
     */
    size_t headerLength() const { return std::distance(m_header, m_data); }

    /**
     * @brief Size of properties data.
     *
     * @return size of properties data.
     */
    int size() const { return m_data[-1]; }

    /**
     * @brief Property data access `operator[]`.
     *
     * @param idx byte index.
     * @return property byte.
     */
    const uint8_t &operator[](int idx) const { return m_data[idx]; }

    /**
     * @brief Available bytes for read.
     *
     * @return int Available bytes for read.
     */
    int available() const { return std::distance(m_data + this->size(), m_end); }

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
    PropertyUUID uuid() const { return m_header[1] * 256 + m_header[0]; }

    /**
     * @brief Advance reader to next property.
     *
     */
    void advance() {
      auto offset = this->size() + this->headerLength();
      m_header += offset;
      m_data += offset;
    }

   private:
    const uint8_t *m_header;
    const uint8_t *m_data;
    const uint8_t *const m_end;
  };

  /**
   * @brief Create properties data reader. Valid only for one read.
   *
   * @return `PropertiesReader` instance.
   */
  PropertiesReader getReader() const;

 protected:
  explicit PropertiesData(uint8_t id) : FrameData{{id, 0}} {}
  explicit PropertiesData(FrameData &&data) : FrameData{std::move(data)} {}
};

}  // namespace midea
}  // namespace dudanov
