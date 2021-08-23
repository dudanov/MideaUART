#pragma once
#include <Arduino.h>
#include "Appliance/ApplianceBase.h"
#include "Appliance/AirConditioner/StatusData.h"
#include "Appliance/AirConditioner/Capabilities.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {
namespace ac {

/// Enum for all modes a Midea device can be in.
enum Mode : uint8_t {
  ///
  MODE_OFF,
  /// The Midea device is set to automatically change the heating/cooling cycle
  MODE_AUTO,
  /// The Midea device is manually set to cool mode (not in auto mode!)
  MODE_COOL,
  /// The Midea device is manually set to dry mode
  MODE_DRY,
  /// The Midea device is manually set to heat mode (not in auto mode!)
  MODE_HEAT,
  /// The Midea device is manually set to fan only mode
  MODE_FAN_ONLY,
};

/// Enum for all modes a Midea fan can be in
enum FanMode : uint8_t {
  /// The fan mode is set to Auto
  FAN_AUTO = 102,
  /// The fan mode is set to Silent
  FAN_SILENT = 20,
  /// The fan mode is set to Low
  FAN_LOW = 40,
  /// The fan mode is set to Medium
  FAN_MEDIUM = 60,
  /// The fan mode is set to High
  FAN_HIGH = 80,
  /// The fan mode is set to Turbo
  FAN_TURBO = 100,
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
  /// The ECO preset
  PRESET_ECO,
  /// The TURBO preset
  PRESET_TURBO,
  /// The SLEEP preset
  PRESET_SLEEP,
  /// The FREEZE_PROTECTION preset
  PRESET_FREEZE_PROTECTION,
};

// Air conditioner control command
struct Control {
  Optional<float> targetTemp{};
  Optional<Mode> mode{};
  Optional<Preset> preset{};
  Optional<FanMode> fanMode{};
  Optional<SwingMode> swingMode{};
};

class AirConditioner : public ApplianceBase {
 public:
  AirConditioner() : ApplianceBase(AIR_CONDITIONER) {}
  void m_setup() override;
  void m_onIdle() override { this->m_getStatus(); }
  void control(const Control &control);
  float getTargetTemp() const { return this->m_targetTemp; }
  float getIndoorTemp() const { return this->m_indoorTemp; }
  float getOutdoorTemp() const { return this->m_outdoorTemp; }
  float getIndoorHum() const { return this->m_indoorHumidity; }
  float getPowerUsage() const { return this->m_powerUsage; }
  Mode getMode() const { return this->m_mode; }
  SwingMode getSwingMode() const { return this->m_swingMode; }
  FanMode getFanMode() const { return this->m_fanMode; }
  Preset getPreset() const { return this->m_preset; }
  const Capabilities &getCapabilities() const { return this->m_capabilities; }
  void displayToggle() { this->m_displayToggle(); }
 protected:
  void m_getPowerUsage();
  void m_getCapabilities();
  void m_getStatus();
  void m_displayToggle();
  ResponseStatus m_readStatus(FrameData data);
  Capabilities m_capabilities{};
  Timer m_powerUsageTimer;
  float m_indoorHumidity{};
  float m_indoorTemp{};
  float m_outdoorTemp{};
  float m_targetTemp{};
  float m_powerUsage{};
  Mode m_mode{};
  Preset m_preset{};
  FanMode m_fanMode{};
  SwingMode m_swingMode{};
  StatusData m_status{};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
