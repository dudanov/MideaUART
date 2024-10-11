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
   * @param s frame to write.
   */
  void write(const Frame &s);

  /**
   * @brief Creates frame instance from parameters and writes it to the stream.
   *
   * @param typeID type of frame.
   * @param s frame data body.
   */
  void write(FrameType typeID, const FrameData &s) { this->write(Frame(m_applianceID, m_protocolID, typeID, s)); }

 protected:
  // IO stream interface instance.
  IOStream *m_io{nullptr};

  // Appliance ID.
  ApplianceID m_applianceID{BROADCAST};

  // Protocol ID.
  uint8_t m_protocolID{};

  // Updates appliance ID.
  void m_updAppID();

  // Updates protocol ID.
  void m_updProtoID();
};

}  // namespace midea
}  // namespace dudanov
