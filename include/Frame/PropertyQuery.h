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
class PropertyQuery : public FrameData {
 public:
  /**
   * @brief Initializes a new frame with type ID.
   * @param id frame type ID.
   */
  PropertyQuery(uint8_t id) : FrameData{{id, 0}} {}

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
   * @brief Properties data reader.
   *
   */
  class PropertiesReader {
   public:
    /**
     * @brief Constructor from `PropertyQuery`. Skip `ID`, `NUM` and `CRC` fields.
     *
     * @param query reference to `PropertyQuery`.
     * @param hdr_size header size.
     */
    explicit PropertiesReader(const PropertyQuery &query, size_t hdr_size)
        : m_data{&query.m_data[3 + hdr_size]}, m_hdr_size{hdr_size}, m_end{&query.m_data.back()} {}

    /**
     * @brief Size of properties data.
     *
     * @return size of properties data.
     */
    int size() const { return this->m_data[-1]; }

    /**
     * @brief Property data access `operator[]`.
     *
     * @param idx byte index.
     * @return property byte.
     */
    uint8_t operator[](int idx) const { return this->m_data[idx]; }

    /**
     * @brief Available bytes for read.
     *
     * @return int Available bytes for read.
     */
    int available() const { return std::distance(this->m_data + this->size(), this->m_end); }

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
    PropertyUUID uuid() const {
      auto it = this->m_data - this->m_hdr_size;
      return it[1] * 256 + it[0];
    }

    /**
     * @brief Advance reader to next property.
     *
     */
    void advance() { this->m_data += this->size() + this->m_hdr_size; }

   private:
    const uint8_t *m_data;
    size_t const m_hdr_size;
    const uint8_t *const m_end;
  };

  /**
   * @brief Create properties data reader. Valid only for one read.
   *
   * @return `PropertiesReader` instance.
   */
  PropertiesReader getReader() const {
    size_t hdr_size = 3;

    if (this->hasID(0xB1))
      hdr_size = 4;

    return PropertiesReader(*this, hdr_size);
  }
};

}  // namespace midea
}  // namespace dudanov
