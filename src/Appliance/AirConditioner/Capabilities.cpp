#include "Appliance/AirConditioner/Capabilities.h"
#include "Frame/FrameData.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "CmdB5";

CmdB5 &CmdB5::setBaseFunc() {
  this->power = true;
  this->mode = true;
  this->temperature = true;
  this->windSpeed = true;
  this->airCheck = false;
  this->unitChangeable = true;
  this->eco = false;
  this->indoor_temperature = false;
  this->turbo = true;
  this->outdoor_temperature = false;
  this->updownFan = false;
  this->leftrightFan = false;
  this->eightHot = false;
  this->cool = true;
  this->hot = true;
  this->dry = true;
  this->auto1 = true;
  this->wind = true;
  this->mutilTemp = true;
  this->powerCal = false;
  this->strongCool = true;
  this->strongHot = false;
  return *this;
}

CmdB5 &CmdB5::toSubCool() {
  this->auto1 = false;
  this->hot = false;
  this->eightHot = false;
  this->cool = true;
  this->dry = false;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->turbo = true;
  this->strongCool = true;
  return *this;
}

CmdB5 &CmdB5::toOnlyCool() {
  this->auto1 = true;
  this->hot = false;
  this->eightHot = false;
  this->cool = true;
  this->dry = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->turbo = true;
  this->strongCool = true;
  return *this;
}

CmdB5 &CmdB5::toOnlyHot() {
  this->auto1 = true;
  this->cool = false;
  this->dry = false;
  this->hot = true;
  this->eightHot = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->turbo = true;
  this->strongCool = true;
  return *this;
}

CmdB5 &CmdB5::toAllEnable() {
  this->auto1 = true;
  this->hot = true;
  this->eightHot = true;
  this->cool = true;
  this->dry = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->turbo = true;
  this->strongCool = true;
  return *this;
}

constexpr unsigned makeU16(uint8_t hi, uint8_t lo) { return 256 * hi + lo; }

enum B5Func : unsigned {
  VERTICAL_WIND = makeU16(0, 9),
  HORIZONTAL_WIND = makeU16(0, 10),
  INDOOR_HUMIDITY = makeU16(0, 21),
  NO_WIND_FEEL = makeU16(0, 24),
  SMART_EYE = makeU16(0, 48),
  BLOWING_PEOPLE = makeU16(0, 50),
  AVOID_PEOPLE = makeU16(0, 51),
  SELF_CLEAN = makeU16(0, 57),
  ONE_KEY_NO_WIND_ON_ME = makeU16(0, 66),
  BREEZE = makeU16(0, 67),
  FRESH_AIR = makeU16(0, 75),
  JET_COOL = makeU16(0, 103),
  NO_WIND_SPEED = makeU16(2, 16),
  ECO_MODES = makeU16(2, 18),
  EIGHT_HOT = makeU16(2, 19),
  MODES = makeU16(2, 20),
  SWING_MODES = makeU16(2, 21),
  POWER_FUNC = makeU16(2, 22),
  NEST = makeU16(2, 23),
  DIANFURE = makeU16(2, 25),
  TURBO_MODES = makeU16(2, 26),
  HUMIDITY = makeU16(2, 31),
  UNIT_CHANGEABLE = makeU16(2, 34),
  LIGHT_TYPE = makeU16(2, 36),
  TEMPERATURES = makeU16(2, 37),
  HAS_BUZZER = makeU16(2, 44),
  IS_TWINS = makeU16(2, 50),
  IS_FOUR_DIRECTION = makeU16(2, 51),
};

class B5Data {
 public:
  B5Data(const FrameData &data) : m_it(data.data() + 1), m_end(data.data() + data.size() - 1) {}
  B5Func id() const { return static_cast<B5Func>(this->m_it[1] * 256 + this->m_it[0]); }
  size_t available() const { return std::distance(this->m_it, this->m_end); }
  size_t size() const { return this->m_it[2]; }
  uint8_t operator[](size_t idx) const { return this->m_it[idx + 3]; }
  void advance() { this->m_it += this->size() + 3; }

 private:
  const uint8_t *m_it;
  const uint8_t *const m_end;
};

static void setFuncEnable(CmdB5 &dst, const B5Data &data) {
  const uint8_t b = data[0];
  switch (data.id()) {
    case B5Func::VERTICAL_WIND:
      dst.hasVerticalWind = b == 1;
      break;
    case B5Func::HORIZONTAL_WIND:
      dst.hasHorizontalWind = b == 1;
      break;
    case B5Func::INDOOR_HUMIDITY:
      dst.hasIndoorHumidity = b != 0;
      break;
    case B5Func::NO_WIND_FEEL:
      dst.hasNoWindFeel = b != 0;
      break;
    case B5Func::SMART_EYE:
      dst.hasSmartEye = b == 1;
      break;
    case B5Func::SELF_CLEAN:
      dst.hasSelfClean = b == 1;
      break;
    case B5Func::FRESH_AIR:
      dst.hasFreshAir = true;
      dst.isFreshAirEnable = b == 1;
      break;
    case B5Func::JET_COOL:
      dst.hasJetCool = true;
      dst.isJetCoolEnable = b == 1;
      break;
    case B5Func::BLOWING_PEOPLE:
      dst.hasBlowingPeople = b == 1;
      break;
    case B5Func::AVOID_PEOPLE:
      dst.hasAvoidPeople = b == 1;
      break;
    case B5Func::ONE_KEY_NO_WIND_ON_ME:
      dst.hasOneKeyNoWindOnMe = b == 1;
      break;
    case B5Func::BREEZE:
      dst.hasBreeze = b == 1;
      break;
    case B5Func::NO_WIND_SPEED:
      dst.hasNoWindSpeed = b == 1;
      dst.hasWindSpeed = b;
      break;
    case B5Func::HUMIDITY:
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
    case B5Func::UNIT_CHANGEABLE:
      dst.unitChangeable = b == 0;
      break;
    case B5Func::HAS_BUZZER:
      dst.hasBuzzer = b != 0;
      break;
    case B5Func::DIANFURE:
      dst.dianfure = b == 1;
      break;
    case B5Func::TURBO_MODES:
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
    case B5Func::LIGHT_TYPE:
      dst.lightType = b;
      break;
    case B5Func::TEMPERATURES:
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
    case B5Func::IS_TWINS:
      dst.isTwins = b == 1;
      break;
    case B5Func::ECO_MODES:
      dst.eco = b == 1;
      dst.special_eco = b == 2;
      break;
    case B5Func::EIGHT_HOT:
      dst.eightHot = b == 1;
      break;
    case B5Func::MODES:
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
    case B5Func::SWING_MODES:
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
    case B5Func::POWER_FUNC:
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
    case B5Func::NEST:
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
    case B5Func::IS_FOUR_DIRECTION:
      dst.isFourDirection = b == 1;
      break;
  }
}

bool CmdB5::read(const FrameData &frame) {
  B5Data caps(frame);

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

void CmdB5::dump() const {
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
