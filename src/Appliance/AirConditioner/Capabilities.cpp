#include "Appliance/AirConditioner/Capabilities.h"
#include "Frame/FrameData.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "Capabilities";

Capabilities &Capabilities::setBaseFunc() {
  this.power = true;
  this.mode = true;
  this.temperature = true;
  this.windSpeed = true;
  this.airCheck = false;
  this.unitChangeable = true;
  this.eco = false;
  this.indoor_temperature = false;
  this.turbo = true;
  this.outdoor_temperature = false;
  this.updownFan = false;
  this.leftrightFan = false;
  this.eightHot = false;
  this.cool = true;
  this.hot = true;
  this.dry = true;
  this.auto = true;
  this.wind = true;
  this.mutilTemp = true;
  this.powerCal = false;
  this.strongCool = true;
  this.strongHot = false;
  return *this;
}

Capabilities &Capabilities::toSubCool() {
  this.auto = false;
  this.hot = false;
  this.eightHot = false;
  this.cool = true;
  this.dry = false;
  this.eco = true;
  this.leftrightFan = true;
  this.unitChangeable = true;
  this.turbo = true;
  this.strongCool = true;
  return *this;
}

Capabilities &Capabilities::toOnlyCool() {
  this.auto = true;
  this.hot = false;
  this.eightHot = false;
  this.cool = true;
  this.dry = true;
  this.eco = true;
  this.leftrightFan = true;
  this.unitChangeable = true;
  this.turbo = true;
  this.strongCool = true;
  return *this;
}

Capabilities &Capabilities::toOnlyHot() {
  this.auto = true;
  this.cool = false;
  this.dry = false;
  this.hot = true;
  this.eightHot = true;
  this.eco = true;
  this.leftrightFan = true;
  this.unitChangeable = true;
  this.turbo = true;
  this.strongCool = true;
  return *this;
}

Capabilities &Capabilities::toAllEnable() {
  this.auto = true;
  this.hot = true;
  this.eightHot = true;
  this.cool = true;
  this.dry = true;
  this.eco = true;
  this.leftrightFan = true;
  this.unitChangeable = true;
  this.turbo = true;
  this.strongCool = true;
  return *this;
}

enum CapabilityID : unsigned {
  VERTICAL_WIND = 0x0009,
  HORIZONTAL_WIND = 0x000A,
  INDOOR_HUMIDITY = 0x0015,
  NO_WIND_FEEL = 0x0018,
  SMART_EYE = 0x0030,
  BLOWING_PEOPLE = 0x0032,
  AVOID_PEOPLE = 0x0033,
  SELF_CLEAN = 0x0039,
  ONE_KEY_NO_WIND_ON_ME = 0x0042,
  BREEZE = 0x0043,
  FRESH_AIR = 0x004B,
  JET_COOL = 0x0067,
  NO_WIND_SPEED = 0x0210,
  ECO_MODES = 0x0212,
  EIGHT_HOT = 0x0213,
  MODES = 0x0214,
  SWING_MODES = 0x0215,
  POWER_FUNC = 0x0216,
  NEST = 0x0217,
  DIANFURE = 0x0219,
  TURBO_MODES = 0x021A,
  HUMIDITY = 0x021F,
  UNIT_CHANGEABLE = 0x0222,
  LIGHT_TYPE = 0x0224,
  TEMPERATURES = 0x0225,
  HAS_BUZZER = 0x022C,
  IS_TWINS = 0x0232,
  IS_FOUR_DIRECTION = 0x0233,
};

class CapabilityData {
 public:
  CapabilityData(const FrameData &data) : m_it(data.data() + 1), m_end(data.data() + data.size() - 1) {}
  CapabilityID id() const { return static_cast<CapabilityID>(this->m_it[1] * 256 + this->m_it[0]); }
  uint8_t size() const { return this->m_it[2]; }
  uint8_t operator[](size_t idx) const { return this->m_it[idx + 3]; }
  size_t available() const { return std::distance(this->m_it, this->m_end); }
  void advance() { this->m_it += this->size() + 3; }

 private:
  const uint8_t *m_it;
  const uint8_t *const m_end;
};

static void setFuncEnable(Capabilities &dst, const CapabilityData &data) {
  const uint8_t b = data[0];
  switch (data.id()) {
    case CapabilityID::VERTICAL_WIND:
      dst.hasVerticalWind = b == 1;
      break;
    case CapabilityID::HORIZONTAL_WIND:
      dst.hasHorizontalWind = b == 1;
      break;
    case CapabilityID::INDOOR_HUMIDITY:
      dst.hasIndoorHumidity = b != 0;
      break;
    case CapabilityID::NO_WIND_FEEL:
      dst.hasNoWindFeel = b != 0;
      break;
    case CapabilityID::SMART_EYE:
      dst.hasSmartEye = b == 1;
      break;
    case CapabilityID::SELF_CLEAN:
      dst.hasSelfClean = b == 1;
      break;
    case CapabilityID::FRESH_AIR:
      dst.hasFreshAir = true;
      dst.isFreshAirEnable = b == 1;
      break;
    case CapabilityID::JET_COOL:
      dst.hasJetCool = true;
      dst.isJetCoolEnable = b == 1;
      break;
    case CapabilityID::BLOWING_PEOPLE:
      dst.hasBlowingPeople = b == 1;
      break;
    case CapabilityID::AVOID_PEOPLE:
      dst.hasAvoidPeople = b == 1;
      break;
    case CapabilityID::ONE_KEY_NO_WIND_ON_ME:
      dst.hasOneKeyNoWindOnMe = b == 1;
      break;
    case CapabilityID::BREEZE:
      dst.hasBreeze = b == 1;
      break;
    case CapabilityID::NO_WIND_SPEED:
      dst.hasNoWindSpeed = b == 1;
      break;
    case CapabilityID::HUMIDITY:
      if (b == 0) {
        dst.hasAutoClearHumidity = false;
        dst.hasHandClearHumidity = false;
      } else if (b == 1) {
        dst.hasAutoClearHumidity = true;
        dst.hasHandClearHumidity = false;
      } else if (b == 2) {
        dst.hasAutoClearHumidity = true;
        dst.hasHandClearHumidity = true;
      } else if (b == 3) {
        dst.hasAutoClearHumidity = false;
        dst.hasHandClearHumidity = true;
      }
      break;
    case CapabilityID::UNIT_CHANGEABLE:
      dst.unitChangeable = b == 0;
      break;
    case CapabilityID::HAS_BUZZER:
      dst.hasBuzzer = b != 0;
      break;
    case CapabilityID::DIANFURE:
      dst.dianfure = b == 1;
      break;
    case CapabilityID::TURBO_MODES:
      if (b == 0) {
        dst.strongHot = false;
        dst.strongCool = true;
      } else if (b == 1) {
        dst.strongHot = true;
        dst.strongCool = true;
      } else if (b == 2) {
        dst.strongHot = false;
        dst.strongCool = false;
      } else if (b == 3) {
        dst.strongHot = true;
        dst.strongCool = false;
      }
      break;
    case CapabilityID::LIGHT_TYPE:
      dst.lightType = b;
      break;
    case CapabilityID::TEMPERATURES:
      dst.cool_adjust_down_temp = b / 2;
      dst.cool_adjust_up_temp = data[1] / 2;
      dst.auto_adjust_down_temp = data[2] / 2;
      dst.auto_adjust_up_temp = data[3] / 2;
      dst.hot_adjust_down_temp = data[4] / 2;
      dst.hot_adjust_up_temp = data[5] / 2;
      if (data.size() > 6)
        dst.isHavePoint = data[6] != 0;
      else
        dst.isHavePoint = data[2] != 0;
      break;
    case CapabilityID::IS_TWINS:
      dst.isTwins = b == 1;
      break;
    case CapabilityID::ECO_MODES:
      dst.eco = b == 1;
      dst.special_eco = b == 2;
      break;
    case CapabilityID::EIGHT_HOT:
      dst.eightHot = b == 1;
      break;
    case CapabilityID::MODES:
      dst.hotcold = b;
      if (b == 1) {
        dst.cool = true;
        dst.hot = true;
        dst.dry = true;
        dst.auto1 = true;
      } else if (b == 2) {
        dst.cool = false;
        dst.dry = false;
        dst.hot = true;
        dst.auto1 = true;
      } else if (b == 3) {
        dst.cool = true;
        dst.dry = false;
        dst.hot = false;
        dst.auto1 = false;
      } else if (b == 4) {
        dst.cool = true;
        dst.dry = false;
        dst.hot = true;
        dst.auto1 = false;
        dst.wind = true;
      } else if (b == 5) {
        dst.cool = true;
        dst.dry = true;
        dst.hot = false;
        dst.auto1 = false;
        dst.wind = true;
      } else {
        dst.hot = false;
        dst.cool = true;
        dst.dry = true;
        dst.auto1 = true;
      }
      break;
    case CapabilityID::SWING_MODES:
      if (b == 0) {
        dst.leftrightFan = false;
        dst.updownFan = true;
      } else if (b == 1) {
        dst.leftrightFan = true;
        dst.updownFan = true;
      } else if (b == 2) {
        dst.leftrightFan = false;
        dst.updownFan = false;
      } else if (b == 3) {
        dst.leftrightFan = true;
        dst.updownFan = false;
      }
      break;
    case CapabilityID::POWER_FUNC:
      if (b == 0 || b == 1) {
        dst.powerCal = false;
        dst.powerCalSetting = false;
        dst.powerCalBCD = true;
      } else if (b == 2) {
        dst.powerCal = true;
        dst.powerCalSetting = false;
        dst.powerCalBCD = true;
      } else if (b == 3) {
        dst.powerCal = true;
        dst.powerCalSetting = true;
        dst.powerCalBCD = true;
      } else if (b == 4) {
        dst.powerCal = true;
        dst.powerCalSetting = false;
        dst.powerCalBCD = false;
      } else if (b == 5) {
        dst.powerCal = true;
        dst.powerCalSetting = true;
        dst.powerCalBCD = false;
      }
      break;
    case CapabilityID::NEST:
      if (b == 0) {
        dst.nestCheck = false;
        dst.nestNeedChange = false;
      } else if (b == 1 || b == 2) {
        dst.nestCheck = true;
        dst.nestNeedChange = false;
      } else if (b == 3) {
        dst.nestCheck = false;
        dst.nestNeedChange = true;
      } else if (b == 4) {
        dst.nestCheck = true;
        dst.nestNeedChange = true;
      }
      break;
    case CapabilityID::IS_FOUR_DIRECTION:
      dst.isFourDirection = b == 1;
      break;
  }
}

bool Capabilities::read(const FrameData &frame) {
  CapabilityData caps(frame);

  for (; caps.available() > 3; caps.advance())
    setFuncEnable(*this, caps);

  if (caps.available() == 3) {
    this->zNum = caps[-2];
    return true;
  }

  this->zNum = 0;
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
