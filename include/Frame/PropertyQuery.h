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

  class PropertiesReaderInternal {
   public:
    /**
     * @brief Size of properties data.
     *
     * @return size of properties data.
     */
    int size() const { return this->m_it[-1]; }

    /**
     * @brief Property data access `operator[]`.
     *
     * @param idx byte index.
     * @return property byte.
     */
    uint8_t operator[](int idx) const { return this->m_it[idx]; }

    /**
     * @brief Available bytes for read.
     *
     * @return int Available bytes for read.
     */
    int available() const { return std::distance(this->m_it, this->m_end); }

    /**
     * @brief Current property is valid.
     *
     * @return property validation result.
     */
    bool valid() const {
      const auto available = this->available();
      return available > 0 && available >= this->size();
    }

   protected:
    PropertiesReaderInternal(const uint8_t *begin, const uint8_t *end) : m_it{begin}, m_end{end} {}
    const uint8_t *m_it;
    const uint8_t *const m_end;
  };

  /**
   * @brief Properties data reader.
   *
   */
  template<int HEADER_SIZE> class PropertiesReader : public PropertiesReaderInternal {
   public:
    /**
     * @brief Constructor from `PropertyQuery`. Skip `ID`, `NUM` and `CRC` fields.
     *
     * @param data reference to `PropertyQuery`.
     */
    explicit PropertiesReader(const PropertyQuery &query)
        : PropertiesReaderInternal{&query.m_data[3 + HEADER_SIZE], &query.m_data.back()} {}

    /**
     * @brief Property UUID.
     *
     * @return UUID.
     */
    PropertyUUID uuid() const {
      auto it = this->m_it - HEADER_SIZE;
      return it[1] * 256 + it[0];
    }

    /**
     * @brief Advance reader to next property.
     *
     */
    void advance() { this->m_it += this->size() + HEADER_SIZE; }
  };

  /**
   * @brief Create properties data reader. Valid only for one read.
   *
   * @return `PropertiesReader` instance.
   */
  template<int HEADER_SIZE> PropertiesReader<HEADER_SIZE> getReader() const {
    return PropertiesReader<HEADER_SIZE>(*this);
  }
};

}  // namespace midea
}  // namespace dudanov
