#pragma once
#include <Arduino.h>
#include <Appliance/AirConditioner/StatusData.h>
#include <set>

namespace dudanov {
namespace midea {
namespace ac {

class Capabilities {
 public:
  // Read from frames
  bool read(const FrameData &data);
  // Dump capabilities
  void dump() const;

  // Control humidity
  bool autoSetHumidity() const { return this->m_autoSetHumidity; };
  bool activeClean() const { return this->m_activeClean; };
  bool breezeControl() const { return this->m_breezeControl; };
  bool buzzer() const { return this->m_buzzer; }
  bool decimals() const { return this->m_decimals; }
  bool electricAuxHeating() const { return this->m_electricAuxHeating; }
  bool fanSpeedControl() const { return this->m_fanSpeedControl; }
  bool indoorHumidity() const { return this->m_indoorHumidity; }
  // Control humidity
  bool manualSetHumidity() const { return this->m_manualSetHumidity; }
  bool nestCheck() const { return this->m_nestCheck; }
  bool nestNeedChange() const { return this->m_nestNeedChange; }
  bool oneKeyNoWindOnMe() const { return this->m_oneKeyNoWindOnMe; }
  bool powerCal() const { return this->m_powerCal; }
  bool powerCalSetting() const { return this->m_powerCalSetting; }
  bool silkyCool() const { return this->m_silkyCool; }
  // Intelligent eye function
  bool smartEye() const { return this->m_smartEye; }
  // Temperature unit can be changed between Celsius and Fahrenheit
  bool unitChangeable() const { return this->m_unitChangeable; }
  bool windOfMe() const { return this->m_windOfMe; }
  bool windOnMe() const { return this->m_windOnMe; }
  
  /* MODES */

  bool supportAutoMode() const { return this->m_autoMode; }
  bool supportCoolMode() const { return this->m_coolMode; }
  bool supportHeatMode() const { return this->m_heatMode; }
  bool supportDryMode() const { return this->m_dryMode; }

  /* PRESETS */

  bool supportFrostProtectionPreset() const { return this->m_frostProtectionMode; }
  bool supportTurboPreset() const { return this->m_turboCool || this->m_turboHeat; }
  bool supportEcoPreset() const { return this->m_ecoMode || this->m_specialEco; }

  /* SWING MODES */

  bool supportVerticalSwing() const { return this->m_updownFan; }
  bool supportHorizontalSwing() const { return this->m_leftrightFan; }
  bool supportBothSwing() const { return this->m_updownFan && this->m_leftrightFan; }

  /* TEMPERATURES */

  float maxTempAuto() const { return this->m_maxTempAuto; }
  float maxTempCool() const { return this->m_maxTempCool; }
  float maxTempHeat() const { return this->m_maxTempHeat; }
  float minTempAuto() const { return this->m_minTempAuto; }
  float minTempCool() const { return this->m_minTempCool; }
  float minTempHeat() const { return this->m_minTempHeat; }

  // Ability to turn LED display off
  bool supportLightControl() const { return this->m_lightControl; }

 protected:
  bool m_updownFan{false};
  bool m_leftrightFan{false};
  bool m_autoMode{false};
  bool m_coolMode{false};
  bool m_dryMode{false};
  bool m_ecoMode{false};
  bool m_specialEco{false};
  bool m_frostProtectionMode{false};
  bool m_heatMode{false};
  bool m_turboCool{false};
  bool m_turboHeat{false};
  bool m_autoSetHumidity{false};
  bool m_activeClean{false};
  bool m_breezeControl{false};
  bool m_buzzer{false};
  bool m_decimals{false};
  bool m_electricAuxHeating{false};
  bool m_fanSpeedControl{true};
  bool m_indoorHumidity{false};
  bool m_lightControl{false};
  bool m_manualSetHumidity{false};
  float m_maxTempAuto{30};
  float m_maxTempCool{30};
  float m_maxTempHeat{30};
  float m_minTempAuto{17};
  float m_minTempCool{17};
  float m_minTempHeat{17};
  bool m_nestCheck{false};
  bool m_nestNeedChange{false};
  bool m_oneKeyNoWindOnMe{false};
  bool m_powerCal{false};
  bool m_powerCalSetting{false};
  bool m_silkyCool{false};
  bool m_smartEye{false};
  bool m_unitChangeable{false};
  bool m_windOfMe{false};
  bool m_windOnMe{false};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
