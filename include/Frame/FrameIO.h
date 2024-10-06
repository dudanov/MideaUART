#pragma once
#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

/**
 * @brief IStream interface.
 *
 */
class IStream {
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

/**
 * @brief Frame reader class.
 *
 */
class FrameIO : public Frame {
 public:
  /**
   * @brief Default constructor.
   *
   */
  explicit FrameIO() = default;

  /**
   * @brief Sets stream interface instance.
   *
   * @param s IO interface.
   */
  void setStream(IStream *s) { m_io = s; }

  /**
   * @brief Clears frame after handling.
   *
   */
  void clear() { m_data.clear(); }

  /**
   * @brief Deserializes frame byte by byte. Caller is responsible for clearing frame after handling.
   *
   * @return `true` if frame deserializing is complete and it ready for handling.
   */
  bool read();

  /**
   * @brief Writes frame to the stream.
   *
   * @param frame frame to write.
   */
  void write(const Frame &frame);

  /**
   * @brief Creates frame instance from parameters and writes it to the stream.
   *
   * @param applianceID appliance type.
   * @param protocolID protocol.
   * @param typeID type of frame.
   * @param s frame data body.
   */
  void write(ApplianceID applianceID, uint8_t protocolID, FrameType typeID, const FrameData &s);

 protected:
  IStream *m_io;
};

}  // namespace midea
}  // namespace dudanov
