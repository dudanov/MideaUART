#pragma once
#include <Arduino.h>
#include "Appliance/ApplianceBase.h"
#include "Appliance/AirConditioner/StatusData.h"
#include "Appliance/AirConditioner/Capabilities.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {
class FrameData;
namespace ac {

// Air conditioner control command
struct Control {
  Optional<float> targetTemp{};
  Optional<Mode> mode{};
  Optional<Preset> preset{};
  Optional<FanMode> fanMode{};
  Optional<SwingMode> swingMode{};
};

enum Properties : uint8_t {
  PROPERTY_LAST_MARKER,
  PROPERTY_MODE,
  PROPERTY_PRESET,
  PROPERTY_FAN_MODE,
  PROPERTY_SWING_MODE,
  PROPERTY_TARGET_TEMP,
  PROPERTY_INDOOR_TEMP,
  PROPERTY_OUTDOOR_TEMP,
  PROPERTY_INDOOR_HUMIDITY,
  PROPERTY_POWER_USAGE,
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
