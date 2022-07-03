#include "Appliance/AirConditioner/Capabilities.h"
#include "Frame/FrameData.h"
#include "Helpers/Helpers.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "CmdB5";

bool CmdB5::isNeedB1Query() {
  return this->hasNoWindFeel || this->hasSelfClean || this->hasOneKeyNoWindOnMe || this->hasBreeze || this->hasBuzzer ||
         this->hasSmartEye || this->hasIndoorHumidity || this->hasVerticalWind || this->hasHorizontalWind ||
         this->isTwins || this->isFourDirection;
}

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
  SILKY_COOL = makeU16(0, 24),
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

class B5Reader {
 public:
  // Constructor from FrameData. Skip ID, NUM and CRC bytes.
  B5Reader(const FrameData &data) : m_it(data.data() + 2), m_end(data.data() + data.size() - 1) {}
  uint16_t getFunction() const { return ByteHelpers::getLE<uint16_t>(this->m_it); }
  size_t available() const { return std::distance(this->m_it, this->m_end); }
  size_t size() const { return this->m_it[2]; }
  uint8_t operator[](size_t idx) const { return this->m_it[idx + 3]; }
  void advance() { this->m_it += this->size() + 3; }

 private:
  const uint8_t *m_it;
  const uint8_t *const m_end;
};

static void setFuncEnable(CmdB5 &dst, const B5Reader &data) {
  const uint8_t b0 = data[0];
  switch (data.getFunction()) {
    case B5Func::VERTICAL_WIND:
      dst.hasVerticalWind = b0 == 1;
      break;
    case B5Func::HORIZONTAL_WIND:
      dst.hasHorizontalWind = b0 == 1;
      break;
    case B5Func::INDOOR_HUMIDITY:
      dst.hasIndoorHumidity = b0 != 0;
      break;
    case B5Func::SILKY_COOL:
      dst.hasNoWindFeel = b0 != 0;
      break;
    case B5Func::SMART_EYE:
      dst.hasSmartEye = b0 == 1;
      break;
    case B5Func::SELF_CLEAN:
      dst.hasSelfClean = b0 == 1;
      break;
    case B5Func::FRESH_AIR:
      dst.hasFreshAir = true;
      dst.isFreshAirEnable = b0 == 1;
      break;
    case B5Func::JET_COOL:
      dst.hasJetCool = true;
      dst.isJetCoolEnable = b0 == 1;
      break;
    case B5Func::BLOWING_PEOPLE:
      dst.hasBlowingPeople = b0 == 1;
      break;
    case B5Func::AVOID_PEOPLE:
      dst.hasAvoidPeople = b0 == 1;
      break;
    case B5Func::ONE_KEY_NO_WIND_ON_ME:
      dst.hasOneKeyNoWindOnMe = b0 == 1;
      break;
    case B5Func::BREEZE:
      dst.hasBreeze = b0 == 1;
      break;
    case B5Func::NO_WIND_SPEED:
      dst.hasNoWindSpeed = b0 == 1;
      dst.hasWindSpeed = b0;
      break;
    case B5Func::HUMIDITY:
      if (b0 == 0) {
        dst.hasAutoClearHumidity = false;
        dst.hasHandClearHumidity = false;
      } else if (b0 == 1) {
        dst.hasAutoClearHumidity = true;
        dst.hasHandClearHumidity = false;
      } else if (b0 == 2) {
        dst.hasAutoClearHumidity = true;
        dst.hasHandClearHumidity = true;
      } else if (b0 == 3) {
        dst.hasAutoClearHumidity = false;
        dst.hasHandClearHumidity = true;
      }
      break;
    case B5Func::UNIT_CHANGEABLE:
      dst.unitChangeable = b0 == 0;
      break;
    case B5Func::HAS_BUZZER:
      dst.hasBuzzer = b0 != 0;
      break;
    case B5Func::DIANFURE:
      dst.dianfure = b0 == 1;
      break;
    case B5Func::TURBO_MODES:
      if (b0 == 0) {
        dst.strongHot = false;
        dst.strongCool = true;
      } else if (b0 == 1) {
        dst.strongHot = true;
        dst.strongCool = true;
      } else if (b0 == 2) {
        dst.strongHot = false;
        dst.strongCool = false;
      } else if (b0 == 3) {
        dst.strongHot = true;
        dst.strongCool = false;
      }
      break;
    case B5Func::LIGHT_TYPE:
      dst.lightType = b0;
      break;
    case B5Func::TEMPERATURES:
      dst.cool_adjust_down_temp = b0 / 2;
      dst.cool_adjust_up_temp = data[1] / 2;
      dst.auto_adjust_down_temp = data[2] / 2;
      dst.auto_adjust_up_temp = data[3] / 2;
      dst.hot_adjust_down_temp = data[4] / 2;
      dst.hot_adjust_up_temp = data[5] / 2;
      dst.isHavePoint = (data.size() > 6) ? data[6] : data[2];
      break;
    case B5Func::IS_TWINS:
      dst.isTwins = b0 == 1;
      break;
    case B5Func::ECO_MODES:
      dst.eco = b0 == 1;
      dst.special_eco = b0 == 2;
      break;
    case B5Func::EIGHT_HOT:
      dst.eightHot = b0 == 1;
      break;
    case B5Func::MODES:
      dst.hotcold = b0;
      if (b0 == 1) {
        dst.cool = true;
        dst.hot = true;
        dst.dry = true;
        dst.auto1 = true;
      } else if (b0 == 2) {
        dst.cool = false;
        dst.dry = false;
        dst.hot = true;
        dst.auto1 = true;
      } else if (b0 == 3) {
        dst.cool = true;
        dst.dry = false;
        dst.hot = false;
        dst.auto1 = false;
      } else if (b0 == 4) {
        dst.cool = true;
        dst.dry = false;
        dst.hot = true;
        dst.auto1 = false;
        dst.wind = true;
      } else if (b0 == 5) {
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
      if (b0 == 0) {
        dst.leftrightFan = false;
        dst.updownFan = true;
      } else if (b0 == 1) {
        dst.leftrightFan = true;
        dst.updownFan = true;
      } else if (b0 == 2) {
        dst.leftrightFan = false;
        dst.updownFan = false;
      } else if (b0 == 3) {
        dst.leftrightFan = true;
        dst.updownFan = false;
      }
      break;
    case B5Func::POWER_FUNC:
      if (b0 == 0 || b0 == 1) {
        dst.powerCal = false;
        dst.powerCalSetting = false;
        dst.powerCalBCD = true;
      } else if (b0 == 2) {
        dst.powerCal = true;
        dst.powerCalSetting = false;
        dst.powerCalBCD = true;
      } else if (b0 == 3) {
        dst.powerCal = true;
        dst.powerCalSetting = true;
        dst.powerCalBCD = true;
      } else if (b0 == 4) {
        dst.powerCal = true;
        dst.powerCalSetting = false;
        dst.powerCalBCD = false;
      } else if (b0 == 5) {
        dst.powerCal = true;
        dst.powerCalSetting = true;
        dst.powerCalBCD = false;
      }
      break;
    case B5Func::NEST:
      if (b0 == 0) {
        dst.nestCheck = false;
        dst.nestNeedChange = false;
      } else if (b0 == 1 || b0 == 2) {
        dst.nestCheck = true;
        dst.nestNeedChange = false;
      } else if (b0 == 3) {
        dst.nestCheck = false;
        dst.nestNeedChange = true;
      } else if (b0 == 4) {
        dst.nestCheck = true;
        dst.nestNeedChange = true;
      }
      break;
    case B5Func::IS_FOUR_DIRECTION:
      dst.isFourDirection = b0 == 1;
      break;
  }
}

bool CmdB5::read(const FrameData &frame) {
  B5Reader data(frame);

  for (; data.available() > 3; data.advance())
    setFuncEnable(*this, data);

  this->zNum = (data.available() == 3) ? data[-3] : 0;
  return this->zNum;
}

#define LOG_CAPABILITY(str, condition) \
  if (condition) \
    LOG_CONFIG(TAG, str);

void CmdB5::dump() const {
  LOG_CONFIG(TAG, "CAPABILITIES REPORT:");
  if (this->auto1) {
    LOG_CONFIG(TAG, "  [x] AUTO MODE");
    LOG_CONFIG(TAG, "      - Min: %d", this->auto_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->auto_adjust_up_temp);
  }
  if (this->cool) {
    LOG_CONFIG(TAG, "  [x] COOL MODE");
    LOG_CAPABILITY("      - Boost", this->strongCool);
    LOG_CONFIG(TAG, "      - Min: %d", this->cool_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->cool_adjust_up_temp);
  }
  if (this->hot) {
    LOG_CONFIG(TAG, "  [x] HEAT MODE");
    LOG_CAPABILITY("      - Boost", this->strongHot);
    LOG_CONFIG(TAG, "      - Min: %d", this->hot_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->hot_adjust_up_temp);
  }
  LOG_CAPABILITY("  [x] Dry Mode", this->dry);
  LOG_CAPABILITY("  [x] Auto Dry", this->hasAutoClearHumidity);
  LOG_CAPABILITY("  [x] Custom Dry", this->hasHandClearHumidity);  // .humidity in command

  LOG_CAPABILITY("  [x] WIND SPEED", this->hasWindSpeed);
  LOG_CAPABILITY("  [x] Indoor Humidity", this->hasIndoorHumidity);  // Indoor humidity in B1 response
  LOG_CAPABILITY("  [x] Decimal Point", this->isHavePoint);
  LOG_CAPABILITY("  [x] UNIT CHANGEABLE", this->unitChangeable);

  LOG_CAPABILITY("  [x] FreshAir", this->hasFreshAir);
  LOG_CAPABILITY("  [x] CoolFlash", this->hasJetCool);
  LOG_CAPABILITY("  [x] Breezeless", this->hasBreeze);
  LOG_CAPABILITY("  [x] Vertical Direction", this->hasVerticalWind);
  LOG_CAPABILITY("  [x] Horizontal Direction", this->hasHorizontalWind);
  LOG_CAPABILITY("  [x] TWINS", this->isTwins);
  LOG_CAPABILITY("  [x] FOUR DIRECTION", this->isFourDirection);
  LOG_CAPABILITY("  [x] Vertical Swing", this->leftrightFan);
  LOG_CAPABILITY("  [x] Horizontal Swing", this->updownFan);
  LOG_CAPABILITY("  [x] Silky Cool", this->hasNoWindFeel);
  LOG_CAPABILITY("  [x] ECO", this->eco);
  LOG_CAPABILITY("  [x] Special ECO", this->special_eco);
  LOG_CAPABILITY("  [x] Frost Protection", this->eightHot);
  LOG_CAPABILITY("  [x] Wind ON me", this->hasBlowingPeople);
  LOG_CAPABILITY("  [x] Wind OFF me", this->hasAvoidPeople);
  LOG_CAPABILITY("  [x] LED", this->lightType);
  LOG_CAPABILITY("  [x] Sound", this->hasBuzzer);
  LOG_CAPABILITY("  [x] Active Clean", this->hasSelfClean);
  LOG_CAPABILITY("  [x] Breeze Away", this->hasOneKeyNoWindOnMe);
  LOG_CAPABILITY("  [x] ECO Intelligent Eye", this->hasSmartEye);
  LOG_CAPABILITY("  [x] Filter Clean Reminder",
                 this->nestCheck);  // dusFull in status. cleanFanTime == 1 in command clear timer
  LOG_CAPABILITY("  [x] NEST NEED CHANGE", this->nestNeedChange);
  LOG_CAPABILITY("  [x] Power Report", this->powerCal);
  LOG_CAPABILITY("  [x] Power Limit", this->powerCalSetting);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
