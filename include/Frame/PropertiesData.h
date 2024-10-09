#pragma once
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {

/**
 * @brief Type of property's 16-bit UUID.
 *
 */
using PropertyUUID = uint16_t;

/**
 * @brief Frame body with properties.
 *
 */
class PropertiesData : public FrameData {
 public:
  /**
   * @brief Makes `PropertiesData` for one of the following properties types `[0xB0, 0xB1, 0xB5]`.
   *
   * @param typeID type ID.
   */
  explicit PropertiesData(uint8_t typeID) : FrameData{{typeID, 0}} {}

  /**
   * @brief Appends property UUID (for 0xB0, 0xB1 and 0xB5 only).
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
};

}  // namespace midea
}  // namespace dudanov
