#include "Appliance/AirConditioner/Capabilities.h"

namespace dudanov {
namespace midea {
namespace ac {

enum CapabilityID : uint16_t {
  CAPABILITY_INDOOR_HUMIDITY = 0x0015,
  CAPABILITY_SILKY_COOL = 0x0018,
  CAPABILITY_SMART_EYE = 0x0030,
  CAPABILITY_WIND_ON_ME = 0x0032,
  CAPABILITY_WIND_OF_ME = 0x0033,
  CAPABILITY_ACTIVE_CLEAN = 0x0039,
  CAPABILITY_ONE_KEY_NO_WIND_ON_ME = 0x0042,
  CAPABILITY_BREEZE_CONTROL = 0x0043,
  CAPABILITY_FAN_SPEED_CONTROL = 0x0210,
  CAPABILITY_PRESET_ECO = 0x0212,
  CAPABILITY_PRESET_FREEZE_PROTECTION = 0x0213,
  CAPABILITY_MODES = 0x0214,
  CAPABILITY_SWING_MODES = 0x0215,
  CAPABILITY_POWER = 0x0216,
  CAPABILITY_NEST = 0x0217,
  CAPABILITY_AUX_ELECTRIC_HEATING = 0x0219,
  CAPABILITY_PRESET_TURBO = 0x021A,
  CAPABILITY_HUMIDITY = 0x021F,
  CAPABILITY_UNIT_CHANGEABLE = 0x0222,
  CAPABILITY_LIGHT_CONTROL = 0x0224,
  CAPABILITY_TEMPERATURES = 0x0225,
  CAPABILITY_BUZZER = 0x022C,
};

static uint16_t read_u16(const uint8_t *data) { return (data[1] << 8) | data[0]; }

class CapabilityData {
 public:
  CapabilityData(const FrameData &data) :
    m_it(data.data() + 2),
    m_end(data.data() + data.size() - 1),
    m_num(*(data.data() + 1)) {}
  // Get capability ID
  CapabilityID id() const { return static_cast<CapabilityID>(read_u16(this->m_it)); }
  // Read-only indexed access to capability data
  const uint8_t &operator[](uint8_t idx) const { return *(this->m_it + idx + 3); }
  // Get size of capability data
  uint8_t size() const { return this->m_it[2]; }
  // 
  bool isValid() const { return m_num && this->m_available() >= 3; }
  // One more request needed
  bool isNeedMore() const { return this->m_available() == 2 && *this->m_it != 0; }
  // Advance to next capability
  void advance() {
    this->m_it += this->size() + 3;
    --this->m_num;
  }
 private:
  uint8_t m_available() const { return std::distance(this->m_it, this->m_end); }
  // Iterator
  const uint8_t *m_it;
  // End of data
  const uint8_t *const m_end;
  // Number of capabilities in answer
  uint8_t m_num;
};

bool Capabilities::read(const FrameData &frame) {
  if (frame.size() < 14)
    return false;

  CapabilityData cap(frame);

  for (; cap.isValid(); cap.advance()) {
    if (!cap.size())
      continue;
    const uint8_t uval = cap[0];
    const bool bval = uval;
    switch (cap.id()) {
      case CAPABILITY_INDOOR_HUMIDITY:
        this->m_indoorHumidity = bval;
        break;
      case CAPABILITY_SILKY_COOL:
        this->m_silkyCool = bval;
        break;
      case CAPABILITY_SMART_EYE:
        this->m_smartEye = bval;
        break;
      case CAPABILITY_WIND_ON_ME:
        this->m_windOnMe = bval;
        break;
      case CAPABILITY_WIND_OF_ME:
        this->m_windOfMe = bval;
        break;
      case CAPABILITY_ACTIVE_CLEAN:
        this->m_activeClean = bval;
        break;
      case CAPABILITY_ONE_KEY_NO_WIND_ON_ME:
        this->m_oneKeyNoWindOnMe = bval;
        break;
      case CAPABILITY_BREEZE_CONTROL:
        this->m_breezeControl = bval;
        break;
      case CAPABILITY_FAN_SPEED_CONTROL:
        this->m_fanSpeedControl = !bval;
        break;
      case CAPABILITY_PRESET_ECO:
        this->m_supportedPresets.insert(PRESET_ECO);
        break;
      case CAPABILITY_PRESET_FREEZE_PROTECTION:
        this->m_supportedPresets.insert(PRESET_FREEZE_PROTECTION);
        break;
      case CAPABILITY_MODES:
        switch (uval) {
          case 0:
            this->m_supportedModes.insert({MODE_OFF, MODE_AUTO, MODE_COOL, MODE_DRY});
            break;
          case 1:
            this->m_supportedModes.insert({MODE_OFF, MODE_AUTO, MODE_COOL, MODE_DRY, MODE_HEAT});
            break;
          case 2:
            this->m_supportedModes.insert({MODE_OFF, MODE_AUTO, MODE_HEAT});
            break;
          case 3:
            this->m_supportedModes.insert({MODE_OFF, MODE_COOL});
            break;
        }
        break;
      case CAPABILITY_SWING_MODES:
        switch (uval) {
          case 0:
            this->m_supportedSwingModes.insert({SWING_OFF, SWING_VERTICAL});
            break;
          case 1:
            this->m_supportedSwingModes.insert({SWING_OFF, SWING_HORIZONTAL, SWING_VERTICAL, SWING_BOTH});
            break;
          case 2:
            this->m_supportedSwingModes.insert(SWING_OFF);
            break;
          case 3:
            this->m_supportedSwingModes.insert({SWING_OFF, SWING_HORIZONTAL});
            break;
        }
        break;
      case CAPABILITY_POWER:
        switch (uval) {
          case 0:
          case 1:
            this->m_powerCal = false;
            this->m_powerCalSetting = false;
            break;
          case 2:
            this->m_powerCal = true;
            this->m_powerCalSetting = false;
            break;
          case 3:
            this->m_powerCal = true;
            this->m_powerCalSetting = true;
            break;
        }
        break;
      case CAPABILITY_NEST:
        switch (uval) {
          case 0:
            this->m_nestCheck = false;
            this->m_nestNeedChange = false;
            break;
          case 1:
          case 2:
            this->m_nestCheck = true;
            this->m_nestNeedChange = false;
            break;
          case 3:
            this->m_nestCheck = false;
            this->m_nestNeedChange = true;
            break;
          case 4:
            this->m_nestCheck = true;
            this->m_nestNeedChange = true;
            break;
        }
        break;
      case CAPABILITY_AUX_ELECTRIC_HEATING:
        this->m_electricAuxHeating = bval;
        break;
      case CAPABILITY_PRESET_TURBO:
        switch (uval) {
          case 0:
          case 1:
          case 3:
            this->m_supportedPresets.insert(PRESET_TURBO);
            break;
        }
        break;
      case CAPABILITY_HUMIDITY:
        switch (uval) {
          case 0:
            this->m_autoSetHumidity = false;
            this->m_manualSetHumidity = false;
            break;
          case 1:
            this->m_autoSetHumidity = true;
            this->m_manualSetHumidity = false;
            break;
          case 2:
            this->m_autoSetHumidity = true;
            this->m_manualSetHumidity = true;
            break;
          case 3:
            this->m_autoSetHumidity = false;
            this->m_manualSetHumidity = true;
            break;
        }
        break;
      case CAPABILITY_UNIT_CHANGEABLE:
        this->m_unitChangeable = !bval;
        break;
      case CAPABILITY_LIGHT_CONTROL:
        this->m_lightControl = bval;
        break;
      case CAPABILITY_TEMPERATURES:
        if (cap.size() >= 6) {
          this->m_minTempCool = static_cast<float>(uval) * 0.5f;
          this->m_maxTempCool = static_cast<float>(cap[1]) * 0.5f;
          this->m_minTempAuto = static_cast<float>(cap[2]) * 0.5f;
          this->m_maxTempAuto = static_cast<float>(cap[3]) * 0.5f;
          this->m_minTempHeat = static_cast<float>(cap[4]) * 0.5f;
          this->m_maxTempHeat = static_cast<float>(cap[5]) * 0.5f;
          this->m_decimals = (cap.size() > 6) ? cap[6] : cap[2];
        }
        break;
      case CAPABILITY_BUZZER:
        this->m_buzzer = bval;
        break;
    }
  }

  // Как минимум указывает на предпоследний элемент (минимум 2 непрочитанных байта)
  if (cap.isNeedMore())
    return true; // если предпоследний байт != 0, то нужно опрашивать дальше

  this->m_isReady = true;
  return false;
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
