#include "Appliance/AirConditioner/Capabilities.h"
#include "Frame/FrameData.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "Capabilities";

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
        this->m_smartEye = uval == 1;
        break;
      case CAPABILITY_WIND_ON_ME:
        this->m_windOnMe = uval == 1;
        break;
      case CAPABILITY_WIND_OF_ME:
        this->m_windOfMe = uval == 1;
        break;
      case CAPABILITY_ACTIVE_CLEAN:
        this->m_activeClean = uval == 1;
        break;
      case CAPABILITY_ONE_KEY_NO_WIND_ON_ME:
        this->m_oneKeyNoWindOnMe = uval == 1;
        break;
      case CAPABILITY_BREEZE_CONTROL:
        this->m_breezeControl = uval == 1;
        break;
      case CAPABILITY_FAN_SPEED_CONTROL:
        this->m_fanSpeedControl = uval != 1;
        break;
      case CAPABILITY_PRESET_ECO:
        this->m_ecoMode = uval == 1;
        this->m_specialEco = uval == 2;
        break;
      case CAPABILITY_PRESET_FREEZE_PROTECTION:
        this->m_frostProtectionMode = uval == 1;
        break;
      case CAPABILITY_MODES:
        switch (uval) {
          case 0:
            this->m_heatMode = false;
            this->m_coolMode = true;
            this->m_dryMode = true;
            this->m_autoMode = true;
            break;
          case 1:
            this->m_coolMode = true;
            this->m_heatMode= true;
            this->m_dryMode = true;
            this->m_autoMode = true;
            break;
          case 2:
            this->m_coolMode = false;
            this->m_dryMode = false;
            this->m_heatMode = true;
            this->m_autoMode = true;
            break;
          case 3:
            this->m_coolMode = true;
            this->m_dryMode = false;
            this->m_heatMode = false;
            this->m_autoMode = false;
            break;
        }
        break;
      case CAPABILITY_SWING_MODES:
        switch (uval) {
          case 0:
            this->m_leftrightFan = false;
            this->m_updownFan = true;
            break;
          case 1:
            this->m_leftrightFan = true;
            this->m_updownFan = true;
            break;
          case 2:
            this->m_leftrightFan = false;
            this->m_updownFan = false;
            break;
          case 3:
            this->m_leftrightFan = true;
            this->m_updownFan = false;
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
            this->m_turboHeat = false;
            this->m_turboCool = true;
            break;
          case 1:
            this->m_turboHeat = true;
            this->m_turboCool = true;
            break;
          case 2:
            this->m_turboHeat = false;
            this->m_turboCool = false;
            break;
           case 3:
            this->m_turboHeat = true;
            this->m_turboCool = false;
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

  return false;
}

#define LOG_CAPABILITY(str, condition) \
  if (condition) \
    LOG_CONFIG(TAG, str);

void Capabilities::dump() const {
  LOG_CONFIG(TAG, "CAPABILITIES REPORT:");
  if (this->m_autoMode) {
    LOG_CONFIG(TAG, "  [x] AUTO MODE");
    LOG_CONFIG(TAG, "      - MIN TEMP: %.1f", this->m_minTempAuto);
    LOG_CONFIG(TAG, "      - MAX TEMP: %.1f", this->m_maxTempAuto);
  }
  if (this->m_coolMode) {
    LOG_CONFIG(TAG, "  [x] COOL MODE");
    LOG_CONFIG(TAG, "      - MIN TEMP: %.1f", this->m_minTempCool);
    LOG_CONFIG(TAG, "      - MAX TEMP: %.1f", this->m_maxTempCool);
  }
  if (this->m_heatMode) {
    LOG_CONFIG(TAG, "  [x] HEAT MODE");
    LOG_CONFIG(TAG, "      - MIN TEMP: %.1f", this->m_minTempHeat);
    LOG_CONFIG(TAG, "      - MAX TEMP: %.1f", this->m_maxTempHeat);
  }
  LOG_CAPABILITY("  [x] DRY MODE", this->m_dryMode);
  LOG_CAPABILITY("  [x] ECO MODE", this->m_ecoMode);
  LOG_CAPABILITY("  [x] SPECIAL ECO", this->m_specialEco);
  LOG_CAPABILITY("  [x] FROST PROTECTION MODE", this->m_frostProtectionMode);
  LOG_CAPABILITY("  [x] TURBO COOL", this->m_turboCool);
  LOG_CAPABILITY("  [x] TURBO HEAT", this->m_turboHeat);
  LOG_CAPABILITY("  [x] FANSPEED CONTROL", this->m_fanSpeedControl);
  LOG_CAPABILITY("  [x] BREEZE CONTROL", this->m_breezeControl);
  LOG_CAPABILITY("  [x] LIGHT CONTROL", this->m_lightControl);
  LOG_CAPABILITY("  [x] UPDOWN FAN", this->m_updownFan);
  LOG_CAPABILITY("  [x] LEFTRIGHT FAN", this->m_leftrightFan);
  LOG_CAPABILITY("  [x] AUTO SET HUMIDITY", this->m_autoSetHumidity);
  LOG_CAPABILITY("  [x] MANUAL SET HUMIDITY", this->m_manualSetHumidity);
  LOG_CAPABILITY("  [x] INDOOR HUMIDITY", this->m_indoorHumidity);
  LOG_CAPABILITY("  [x] POWER CAL", this->m_powerCal);
  LOG_CAPABILITY("  [x] POWER CAL SETTING", this->m_powerCalSetting);
  LOG_CAPABILITY("  [x] BUZZER", this->m_buzzer);
  LOG_CAPABILITY("  [x] ACTIVE CLEAN", this->m_activeClean);
  LOG_CAPABILITY("  [x] DECIMALS", this->m_decimals);
  LOG_CAPABILITY("  [x] ELECTRIC AUX HEATING", this->m_electricAuxHeating);
  LOG_CAPABILITY("  [x] NEST CHECK", this->m_nestCheck);
  LOG_CAPABILITY("  [x] NEST NEED CHANGE", this->m_nestNeedChange);
  LOG_CAPABILITY("  [x] ONE KEY NO WIND ON ME", this->m_oneKeyNoWindOnMe);
  LOG_CAPABILITY("  [x] SILKY COOL", this->m_silkyCool);
  LOG_CAPABILITY("  [x] SMART EYE", this->m_smartEye);
  LOG_CAPABILITY("  [x] UNIT CHANGEABLE", this->m_unitChangeable);
  LOG_CAPABILITY("  [x] WIND OF ME", this->m_windOfMe);
  LOG_CAPABILITY("  [x] WIND ON ME", this->m_windOnMe);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
