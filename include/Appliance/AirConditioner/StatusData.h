/**
 * @file StatusData.h
 * @author Sergey V. DUDANOV (sergey.dudanov@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once
#include <Arduino.h>

#include "Frame/PropertiesData.h"
#include "Frame/PropertiesReader.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {
namespace ac {

class Capabilities;

/**
 * @brief All modes a Midea device can be in.
 *
 */
enum Mode : uint8_t {
  ///
  MODE_OFF,
  /// The Midea device is set to automatically change the heating/cooling cycle
  MODE_AUTO,
  /// The Midea device is manually set to cool mode
  MODE_COOL,
  /// The Midea device is manually set to dry mode
  MODE_DRY,
  /// The Midea device is manually set to heat mode
  MODE_HEAT,
  /// The Midea device is manually set to fan only mode
  MODE_FAN_ONLY,
  /// The Midea device is manually set to smart dry mode (in .humidity must be setpoint)
  MODE_DRY_SMART,
};

/**
 * @brief Fan speeds presets enumeration.
 *
 * 1. The fan speed is coded as a percentage. For `AUTO` mode used `102`.
 * 2. As a rule, devices do not support arbitrary speed setting and are represented by a small set of presets.
 * 3. The fan speed is always `AUTO` in `AUTO` and `DRY` operating modes and cannot be changed.
 * 4. `hasWindSpeed` value from 0 to 7.
 *
 * | Flag / Value | 0,5,6 |   1   |   2   |   3   |   4   |   7   |
 * |--------------|-------|-------|-------|-------|-------|-------|
 * | AUTO         | true  | false | false | false | true  | false |
 * | MEDIUM       | true  | false | false | false | false | true  |
 * | ONE_LOW      | false | false | true  | false | false | false |
 * | PERCENT      | false | true  | false | false | false | false |
 *
 *
 *
 * | Speed Preset | Write |  Read   |
 * |--------------|-------|---------|
 * | FAN_SILENT   |    20 | 20      |
 * | FAN_LOW      |    40 | 40 (30) |
 * | FAN_MEDIUM   |    60 | 60 (50) |
 * | FAN_HIGH     |    80 | 80      |
 * | FAN_TURBO    |   100 | 100     |
 * | FAN_FIXED    |   101 | 101     |
 * | FAN_AUTO     |   102 | 102     |
 *
 */
enum FanSpeed : uint8_t {
  FAN_SILENT = 20,
  FAN_LOW = 40,
  FAN_MEDIUM = 60,
  FAN_HIGH = 80,
  FAN_TURBO = 100,
  FAN_FIXED = 101,
  FAN_AUTO = 102,
};

/// Enum for all modes a Midea swing can be in
enum SwingMode : uint8_t {
  /// The sing mode is set to Off
  SWING_OFF = 0b0000,
  /// The fan mode is set to Both
  SWING_BOTH = 0b1111,
  /// The fan mode is set to Vertical
  SWING_VERTICAL = 0b1100,
  /// The fan mode is set to Horizontal
  SWING_HORIZONTAL = 0b0011,
};

/// Enum for all presets a Midea can be in
enum Preset : uint8_t {
  /// None preset
  PRESET_NONE,
  /// The SLEEP preset
  PRESET_SLEEP,
  /// The TURBO preset
  PRESET_TURBO,
  /// The ECO preset
  PRESET_ECO,
  /// The FREEZE_PROTECTION preset
  PRESET_FREEZE_PROTECTION,
};

class StatusData : public FrameData {
 public:
  explicit StatusData()
      : FrameData{0x40, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} {}
  StatusData(const FrameData &data) : FrameData(data) {}

  /// Copy status from another StatusData
  void copyStatus(const StatusData &p) { memcpy(m_data.data() + 1, p.m_data.data() + 1, 10); }

  /* TARGET TEMPERATURE */
  float getTargetTemp() const;
  void setTargetTemp(float temp);

  /* MODE */
  Mode getRawMode() const { return static_cast<Mode>(m_getValue(2, 7, 5)); }
  Mode getMode() const;
  void setMode(Mode mode);

  /* FAN SPEED */
  FanSpeed getFanMode() const;
  void setFanMode(FanSpeed mode) { m_setValue(3, mode); };

  /* SWING MODE */
  SwingMode getSwingMode() const { return static_cast<SwingMode>(m_getValue(7, 15)); }
  void setSwingMode(SwingMode mode) { m_setValue(7, 0x30 | mode); }

  /* INDOOR TEMPERATURE */
  float getIndoorTemp() const;

  /* OUTDOOR TEMPERATURE */
  float getOutdoorTemp() const;

  /* HUMIDITY SETPOINT */
  float getHumiditySetpoint() const;

  /* PRESET */
  Preset getPreset() const;
  void setPreset(Preset preset);

  /* POWER USAGE */
  float getPowerUsage() const;

  void setBeeper(bool state) {
    m_setBit(1, 1, true);
    m_setBit(1, 6, state);
  }

 protected:
  /* POWER */
  bool m_getPower() const { return m_getBit(1, 0); }
  void m_setPower(bool state) { m_setBit(1, 0, state); }
  /* ECO MODE */
  bool m_getEco() const { return m_getBit(9, 4); }
  void m_setEco(bool state) { m_setBit(9, 7, state); }
  /* TURBO MODE */
  bool m_getTurbo() const { return m_getBit(8, 5) || m_getBit(10, 1); }
  void m_setTurbo(bool state) {
    m_setBit(8, 5, state);
    m_setBit(10, 1, state);
  }
  /* FREEZE PROTECTION */
  bool m_getFreezeProtection() const { return m_getBit(21, 7); }
  void m_setFreezeProtection(bool state) { m_setBit(21, 7, state); }
  /* SLEEP MODE */
  bool m_getSleep() const { return m_getBit(10, 0); }
  void m_setSleep(bool state) { m_setBit(10, 0, state); }
};

enum QueryTypes : uint8_t {
  QUERY_STATE,
  QUERY_POWER_USAGE,
  QUERY_DISPLAY_TOGGLE,
  QUERY_CAPABILITIES,
};

/**
 * @brief StateQuery `0x41`.
 *
 */
class StateQuery : public FrameData {
 public:
  explicit StateQuery()
      : FrameData{0x41, 0x81, 0x00, 0xFF, 0x03, 0xFF, 0x00, 0x02, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, m_genID()} {
    this->appendCRC();
  }
};

/**
 * @brief PowerUsageQuery `0x41`.
 *
 */
class PowerUsageQuery : public FrameData {
 public:
  explicit PowerUsageQuery()
      : FrameData{0x41, 0x21, 0x01, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, m_genID()} {
    this->appendCRC();
  }
};

/**
 * @brief DisplayToggleQuery `0x41`.
 *
 */
class DisplayToggleQuery : public FrameData {
 public:
  explicit DisplayToggleQuery()
      : FrameData{0x41, 0x61, 0x00, 0xFF, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, m_genID()} {
    this->appendCRC();
  }
};

/**
 * @brief CapabilitiesQuery `0xB5`.
 *
 */
class CapabilitiesQuery : public FrameData {
 public:
  explicit CapabilitiesQuery() : FrameData{0xB5, 0x01, 0x11} { this->appendCRC(); }
  explicit CapabilitiesQuery(uint8_t idx) : FrameData{0xB5, 0x01, 0x01, idx} { this->appendCRC(); }
};

/**
 * @brief PropertiesStateQuery `0xB1`.
 *
 */
class PropertiesStateQuery : public PropertiesData {
 public:
  // DataBodyQueryB1
  explicit PropertiesStateQuery();
  // DataBodyDevB1
  explicit PropertiesStateQuery(const Capabilities &s);
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
