#pragma once
#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

class IOStream;

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
   * @param io IO interface.
   */
  void setStream(IOStream *io) { m_io = io; }

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
  void write(ApplianceID applianceID, uint8_t protocolID, FrameType typeID, const FrameData &s) {
    this->write(Frame{applianceID, protocolID, typeID, s});
  }

 protected:
  IOStream *m_io;
};

}  // namespace midea
}  // namespace dudanov
