#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {

/**
 * @brief
 *
 */
enum ApplianceID : uint8_t {
  APP_DEHUMIDIFIER = 0xA1, /**< Dehumidifier */
  APP_CONDITIONER = 0xAC,  /**< Air conditioner */
  APP_AIR2WATER = 0xC3,    /**< Air to water */
  APP_FAN = 0xFA,          /**< Fan */
  APP_CLEANER = 0xFC,      /**< Vacuum cleaner */
  APP_HUMIDIFIER = 0xFD,   /**< Humidifier */
  APP_BROADCAST = 0xFF,    /**< Broadcast */
};

/**
 * @brief
 *
 */
enum FrameType : uint8_t {
  DEVICE_CONTROL = 0x02,    /**< Dehumidifier */
  DEVICE_QUERY = 0x03,      /**< Dehumidifier */
  GET_ELECTRONIC_ID = 0x07, /**< Dehumidifier */
  NETWORK_NOTIFY = 0x0D,    /**< Dehumidifier */
  NETWORK_QUERY = 0x63,     /**< Dehumidifier */
};

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
  explicit Frame(ApplianceID applianceID, uint8_t protocolID, FrameType typeID, const FrameData &s);

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

  // Constants
  enum : uint8_t {
    IDX_LENGTH = 1,
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
