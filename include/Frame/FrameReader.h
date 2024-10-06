#pragma once
#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

/**
 * @brief Frame reader class.
 *
 */
class FrameReader : public Frame {
 public:
  /**
   * @brief Default constructor.
   *
   */
  explicit FrameReader() = default;

  /**
   * @brief Clears frame after handling.
   *
   */
  void clear() { m_data.clear(); }

  /**
   * @brief Deserializes frame byte by byte. Caller is responsible for clearing frame after handling.
   *
   * @param data byte to process.
   * @return `true` if frame deserializing is complete and it ready for handling.
   */
  bool deserialize(const uint8_t &data);
};

}  // namespace midea
}  // namespace dudanov
