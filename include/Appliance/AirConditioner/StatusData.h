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

#include "Frame/FrameData.h"
#include "Frame/PropertyQuery.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {
namespace ac {

class CmdB5;

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
  /// The Midea device is manually set to custom dry mode
  MODE_DRY_CUSTOM,
};

/**
 * @brief Fan speeds enum.
 *
 * 1. Fan speed setting not supported in `AUTO` and `DRY` modes. Always `AUTO`.
 * 2. `hasWindSpeed` value from 0 to 7.
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
 * | Enumerator | Write |  Read   |
 * |------------|-------|---------|
 * | FAN_SILENT |    20 | 20      |
 * | FAN_LOW    |    40 | 40 (30) |
 * | FAN_MEDIUM |    60 | 60 (50) |
 * | FAN_HIGH   |    80 | 80      |
 * | FAN_TURBO  |   100 | 100     |
 * | FAN_AUTO   |   102 | 102     |
 *
 */
enum FanSpeed : uint8_t {
  FAN_SILENT = 20,
  FAN_LOW = 40,
  FAN_MEDIUM = 60,
  FAN_HIGH = 80,
  FAN_TURBO = 100,
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
  StatusData()
      : FrameData({0x40, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}
  StatusData(const FrameData &data) : FrameData(data) {}

  /// Copy status from another StatusData
  void copyStatus(const StatusData &p) { memcpy(this->m_data.data() + 1, p.data() + 1, 10); }

  /* TARGET TEMPERATURE */
  float getTargetTemp() const;
  void setTargetTemp(float temp);

  /* MODE */
  Mode getRawMode() const { return static_cast<Mode>(this->m_getValue(2, 7, 5)); }
  Mode getMode() const;
  void setMode(Mode mode);

  /* FAN SPEED */
  FanSpeed getFanMode() const;
  void setFanMode(FanSpeed mode) { this->m_setValue(3, mode); };

  /* SWING MODE */
  SwingMode getSwingMode() const { return static_cast<SwingMode>(this->m_getValue(7, 15)); }
  void setSwingMode(SwingMode mode) { this->m_setValue(7, 0x30 | mode); }

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
    this->m_setMask(1, true, 2);
    this->m_setMask(1, state, 64);
  }

 protected:
  /* POWER */
  bool m_getPower() const { return this->m_getValue(1, 1); }
  void m_setPower(bool state) { this->m_setMask(1, state, 1); }
  /* ECO MODE */
  bool m_getEco() const { return this->m_getValue(9, 16); }
  void m_setEco(bool state) { this->m_setMask(9, state, 128); }
  /* TURBO MODE */
  bool m_getTurbo() const { return this->m_getValue(8, 32) || this->m_getValue(10, 2); }
  void m_setTurbo(bool state) {
    this->m_setMask(8, state, 32);
    this->m_setMask(10, state, 2);
  }
  /* FREEZE PROTECTION */
  bool m_getFreezeProtection() const { return this->m_getValue(21, 128); }
  void m_setFreezeProtection(bool state) { this->m_setMask(21, state, 128); }
  /* SLEEP MODE */
  bool m_getSleep() const { return this->m_getValue(10, 1); }
  void m_setSleep(bool state) { this->m_setMask(10, state, 1); }
};

/// Specific DeviceStateQuery 0x41 frame
class FrameDataDevQuery41 : public FrameData {
 public:
  FrameDataDevQuery41()
      : FrameData({0x41, 0x81, 0x00, 0xFF, 0x03, 0xFF, 0x00, 0x02, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04}) {
    this->appendCRC();
  }
};

/// Specific PowerQuery 0x41 frame
class FrameDataQuery41 : public FrameData {
 public:
  FrameDataQuery41()
      : FrameData({0x41, 0x21, 0x01, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}) {
    this->appendCRC();
  }
};

/// Specific DisplayToggle 0x41 command frame
class FrameDataLight41 : public FrameData {
 public:
  FrameDataLight41()
      : FrameData({0x41, 0x61, 0x00, 0xFF, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<uint8_t>(random(100) + 1)}) {
    this->appendCRC();
  }
};

/// Starting B5Query frame
class FrameDataB5Query : public FrameData {
 public:
  FrameDataB5Query() : FrameData({0xB5, 0x01, 0x11}) { this->appendCRC(); }
};

class FrameDataSecondB5Query : public FrameData {
 public:
  explicit FrameDataSecondB5Query(uint8_t idx) : FrameData({0xB5, 0x01, 0x01, idx}) { this->appendCRC(); }
};

class GetFeatureStateQuery : public PropertyQuery {
 public:
  // DataBodyQueryB1
  GetFeatureStateQuery();
  // DataBodyDevB1
  explicit GetFeatureStateQuery(const CmdB5 &b5);
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
