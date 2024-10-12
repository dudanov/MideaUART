#pragma once
#include "Frame/FrameData.h"
#include <string>

namespace dudanov {
namespace midea {

/**
 * @brief Appliance types ID.
 *
 */
enum ApplianceID : uint8_t {
  DEHUMIDIFIER = 0xA1,    /**< Dehumidifier */
  AIR_CONDITIONER = 0xAC, /**< Air conditioner */
  AIR2WATER = 0xC3,       /**< Air to water */
  FAN = 0xFA,             /**< Fan */
  CLEANER = 0xFC,         /**< Vacuum cleaner */
  HUMIDIFIER = 0xFD,      /**< Humidifier */
  BROADCAST = 0xFF,       /**< Broadcast */
};

/**
 * @brief Frame types.
 *
 */
enum FrameType : uint8_t {
  DEV_CTRL = 2,      /**< Device control */
  DEV_QUERY,         /**< Device query */
  ELECTRONIC_ID = 7, /**< Electronic ID */
  NET_NOTIFY = 13,   /**< Network notify */
  NET_QUERY = 99,    /**< Network query */
};

/**
 * @brief Frame class.
 *
 */
class Frame {
  friend class FrameReaderWriter;

 public:
  /**
   * @brief Makes `Frame` instance from parameters.
   *
   * @param appID appliance ID.
   * @param protoID protocol ID.
   * @param typeID frame type ID.
   * @param s frame data body.
   * @return `Frame` instance.
   */
  explicit Frame(ApplianceID appID, uint8_t protoID, FrameType typeID, const FrameData &s);

  /**
   * @brief Extracts data body from frame. Frame MUST BE full and valid.
   *
   * @return Frame data body.
   */
  FrameData data() const;

  /**
   * @brief Check frame type.
   *
   * @param value frame type.
   * @return `true` if frame has specified type.
   */
  bool hasTypeID(FrameType typeID) const { return m_data[IDX_TYPE] == typeID; }

  /**
   * @brief Protocol ID.
   *
   * @return protocol ID.
   */
  uint8_t protocolID() const { return m_data[IDX_PROTOCOL]; }

  /**
   * @brief Print frame as hex string.
   *
   * @param sep bytes separator.
   * @return Hex string.
   */
  std::string toString(char sep = ' ') const;

 protected:
  // Raw data vector.
  std::vector<uint8_t> m_data;

  /**
   * @brief Protected default constructor. Used for `FrameReaderWriter` instance construction.
   *
   */
  explicit Frame() = default;

  // Length field (size of frame without start byte).
  uint8_t m_len() const { return m_data[IDX_LENGTH]; }

  // Calculates checksum.
  uint8_t m_calcCS() const;

  // Constants
  enum : uint8_t {
    IDX_START,
    IDX_LENGTH,
    IDX_APPLIANCE,
    IDX_SYNC,
    IDX_PROTOCOL = 8,
    IDX_TYPE,
    IDX_DATA,
    SYM_START = 170,
  };
};

}  // namespace midea
}  // namespace dudanov
