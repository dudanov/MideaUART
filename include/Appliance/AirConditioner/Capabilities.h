#pragma once
#include <Arduino.h>
#include <Appliance/AirConditioner/StatusData.h>
#include <set>

namespace dudanov {
namespace midea {

class FrameData;

namespace ac {

class Capabilities {
 public:
  // Read from frames
  bool read(const FrameData &data);
  // Is ready for reading Climate traits
  bool isReady() const { return this->m_isReady; }
  // Control humidity
  bool autoSetHumidity() const { return this->m_autoSetHumidity; };
  bool activeClean() const { return this->m_activeClean; };
  bool breezeControl() const { return this->m_breezeControl; };
  bool buzzer() const { return this->m_buzzer; }
  bool decimals() const { return this->m_decimals; }
  bool electricAuxHeating() const { return this->m_electricAuxHeating; }
  bool fanSpeedControl() const { return this->m_fanSpeedControl; }
  bool indoorHumidity() const { return this->m_indoorHumidity; }
  // Ability to turn LED display off
  bool lightControl() const { return this->m_lightControl; }
  // Control humidity
  bool manualSetHumidity() const { return this->m_manualSetHumidity; }
  float maxTempAuto() const { return this->m_maxTempAuto; }
  float maxTempCool() const { return this->m_maxTempCool; }
  float maxTempHeat() const { return this->m_maxTempHeat; }
  float minTempAuto() const { return this->m_minTempAuto; }
  float minTempCool() const { return this->m_minTempCool; }
  float minTempHeat() const { return this->m_minTempHeat; }
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

 protected:
  std::set<Mode> m_supportedModes;
  std::set<FanMode> m_supportedFanModes;
  std::set<SwingMode> m_supportedSwingModes;
  std::set<Preset> m_supportedPresets;
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
  // is filled?
  bool m_isReady{false};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
