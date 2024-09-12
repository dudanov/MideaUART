#pragma once
#include <cstdint>

namespace dudanov {
namespace midea {

class FrameData;
class PropertiesReader;

/**
 * @brief Capabilities base class.
 *
 */
class CapabilitiesBase {
 public:
  /**
   * @brief Reads capabilities from 0xB5 frame data.
   *
   * @param s Reference to `PropertiesData` instance.
   * @return If zero - no more data is required. Else `ID` of next frame.
   */
  uint8_t read(const FrameData &s);

  /**
   * @brief Dump capabilities report to logs.
   *
   */
  virtual void dump() const = 0;

  /**
   * @brief Checks if any of 0xB1 capabilities is supported.
   *
   * @return `true` if any of 0xB1 capabilities is supported.
   */
  virtual bool isNeedB1Query() const = 0;

 protected:
  virtual void m_setCapability(const PropertiesReader &s) = 0;
};

}  // namespace midea
}  // namespace dudanov
