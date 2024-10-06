#pragma once
#include <cstddef>
#include <cstdint>

namespace dudanov {
namespace midea {

/**
 * @brief IOStream interface.
 *
 */
class IOStream {
 public:
  /**
   * @brief Reads new byte from stream if available.
   *
   * @param dst destination byte.
   * @return `true` if new byte available.
   */
  virtual bool read(uint8_t &dst) = 0;

  /**
   * @brief Writes byte array to the stream.
   *
   * @param src pointer to array.
   * @param len length of array.
   */
  virtual void write(const uint8_t *src, size_t len) = 0;
};

}  // namespace midea
}  // namespace dudanov
