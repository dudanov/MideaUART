#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {

class Frame {
 public:
  /**
   * @brief Makes `Frame` instance from parameters.
   *
   * @param applianceID appliance ID.
   * @param protocolID protocol ID.
   * @param typeID frame type ID.
   * @param s frame data body.
   * @return `Frame` instance.
   */
  explicit Frame(uint8_t applianceID, uint8_t protocolID, uint8_t typeID, const FrameData &s);

  /**
   * @brief Default constructor. Used for deserializer instance construction.
   *
   */
  explicit Frame() = default;

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

  /**
   * @brief Extracts data body from frame. Frame MUST BE full and valid.
   *
   * @return Frame data body.
   */
  FrameData getData() const;

  /**
   * @brief Sets new frame data body and finalize frame by updating checksum.
   *
   * @param s frame data body.
   */
  void setData(const FrameData &s);

  /**
   * @brief Get frame raw data pointer.
   *
   * @return Raw data pointer.
   */
  const uint8_t *data() const { return m_data.data(); }

  /**
   * @brief Get raw data size.
   *
   * @return Raw data size.
   */
  size_t size() const { return m_data.size(); }

  /**
   * @brief Check frame type.
   *
   * @param value frame type.
   * @return `true` if frame has specified type.
   */
  bool hasTypeID(uint8_t typeID) const { return m_data[IDX_TYPE] == typeID; }

  /**
   * @brief Protocol ID.
   *
   * @return protocol ID.
   */
  uint8_t protocolID() const { return m_data[IDX_PROTOCOL]; }

  /**
   * @brief Print frame as hex string.
   *
   * @return Hex string.
   */
  std::string toString() const;

 protected:
  // Raw data vector.
  std::vector<uint8_t> m_data;

  // Length field (size of frame without start byte).
  uint8_t m_len() const { return m_data[IDX_LENGTH]; }

  // Calculates checksum.
  uint8_t m_calcCS() const;

  static const uint8_t SYM_START = 0xAA;
  static const uint8_t IDX_START = 0;
  static const uint8_t IDX_LENGTH = 1;
  static const uint8_t IDX_APPLIANCE = 2;
  static const uint8_t IDX_SYNC = 3;
  static const uint8_t IDX_PROTOCOL = 8;
  static const uint8_t IDX_TYPE = 9;
  static const uint8_t IDX_DATA = 10;
};

}  // namespace midea
}  // namespace dudanov
