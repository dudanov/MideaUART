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
  class ResponseReader;

  /**
   * @brief Create properties data reader. Valid only for one read.
   *
   * @return `ResponseReader` instance.
   */
  ResponseReader getReader() const;
};

class PropertyQuery::ResponseReader {
 public:
  /**
   * @brief Constructor from `PropertyQuery`. Skip `ID`, `NUM` and `CRC` fields.
   *
   * @param data reference to `PropertyQuery`.
   */
  explicit ResponseReader(const PropertyQuery &query) : m_it{&query.m_data[2]}, m_end{&query.m_data.back()} {}

  ResponseReader() = delete;

  /**
   * @brief Property UUID.
   *
   * @return UUID.
   */
  PropertyUUID uuid() const { return m_it[1] * 256 + m_it[0]; }

  /**
   * @brief Size of properties data.
   *
   * @return size of properties data.
   */
  size_t size() const { return this->m_it[2]; }

  /**
   * @brief Property data access `operator[]`.
   *
   * @param idx byte index.
   * @return property byte.
   */
  uint8_t operator[](int idx) const { return this->m_it[idx + 3]; }

  /**
   * @brief Current property is valid.
   *
   * @return property validation result.
   */
  bool valid() const {
    const auto available = std::distance(this->m_it + 3, this->m_end);
    return available >= 0 && available >= this->size();
  }

  /**
   * @brief Advance reader to next property.
   *
   */
  void advance() { this->m_it += this->size() + 3; }

 private:
  const uint8_t *m_it;
  const uint8_t *const m_end;
};

}  // namespace midea
}  // namespace dudanov
