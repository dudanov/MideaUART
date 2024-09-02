#include "Appliance/AirConditioner/Capabilities.h"
#include "Helpers/Helpers.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "CmdB5";

bool CmdB5::isNeedB1Query() const {
  return this->hasNoWindFeel || this->hasSelfClean || this->hasOneKeyNoWindOnMe || this->hasBreeze || this->hasBuzzer ||
         this->hasSmartEye || this->hasIndoorHumidity || this->hasVerticalWind || this->hasHorizontalWind ||
         this->isTwins || this->isFourDirection;
}

void CmdB5::setBaseFunc() {
  this->unitChangeable = true;
  this->eco = false;
  this->updownFan = false;
  this->leftrightFan = false;
  this->eightHot = false;
  this->cool = true;
  this->hot = true;
  this->dry = true;
  this->auto_ = true;
  this->wind = true;
  this->powerCal = false;
  this->strongCool = true;
  this->strongHot = false;
}

void CmdB5::toSubCool() {
  this->auto_ = false;
  this->hot = false;
  this->eightHot = false;
  this->cool = true;
  this->dry = false;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->strongCool = true;
}

void CmdB5::toOnlyCool() {
  this->auto_ = true;
  this->hot = false;
  this->eightHot = false;
  this->cool = true;
  this->dry = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->strongCool = true;
}

void CmdB5::toOnlyHot() {
  this->auto_ = true;
  this->cool = false;
  this->dry = false;
  this->hot = true;
  this->eightHot = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->strongCool = true;
}

void CmdB5::toAllEnable() {
  this->auto_ = true;
  this->hot = true;
  this->eightHot = true;
  this->cool = true;
  this->dry = true;
  this->eco = true;
  this->leftrightFan = true;
  this->unitChangeable = true;
  this->strongCool = true;
}

void CmdB5::m_setFuncEnable(const PropertiesData::PropertiesReader &reader) {
  const uint8_t b0 = reader[0];

  switch (reader.uuid()) {
    case Feature::V_WIND_DIRECTION:
      this->hasVerticalWind = b0 == 1;
      break;

    case Feature::H_WIND_DIRECTION:
      this->hasHorizontalWind = b0 == 1;
      break;

    case Feature::INDOOR_HUMIDITY:
      this->hasIndoorHumidity = b0 != 0;
      break;

    case Feature::SILKY_COOL:
      this->hasNoWindFeel = b0 != 0;
      break;

    case Feature::ECO_EYE:
      this->hasSmartEye = b0 == 1;
      break;

    case Feature::ACTIVE_CLEAN:
      this->hasSelfClean = b0 == 1;
      break;

    case Feature::WIND_ON_ME:
      this->hasBlowingPeople = b0 == 1;
      break;

    case Feature::WIND_OFF_ME:
      this->hasAvoidPeople = b0 == 1;
      break;

    case Feature::BREEZE_AWAY:
      this->hasOneKeyNoWindOnMe = b0 == 1;
      break;

    case Feature::BREEZE:
      this->hasBreeze = b0 == 1;
      break;

    case Feature::FAN_SPEED:
      this->hasNoWindSpeed = b0 == 1;
      this->hasWindSpeed = b0;
      break;

    case Feature::HUMIDITY:
      if (b0 == 0) {
        this->hasAutoClearHumidity = false;
        this->hasHandClearHumidity = false;

      } else if (b0 == 1) {
        this->hasAutoClearHumidity = true;
        this->hasHandClearHumidity = false;

      } else if (b0 == 2) {
        this->hasAutoClearHumidity = true;
        this->hasHandClearHumidity = true;

      } else if (b0 == 3) {
        this->hasAutoClearHumidity = false;
        this->hasHandClearHumidity = true;
      }
      break;

    case Feature::UNIT_CHANGEABLE:
      this->unitChangeable = b0 == 0;
      break;

    case Feature::BUZZER:
      this->hasBuzzer = b0 != 0;
      break;

    case Feature::AUX_HEATER:
      this->dianfure = b0 == 1;
      break;

    case Feature::TURBO_MODES:
      if (b0 == 0) {
        this->strongHot = false;
        this->strongCool = true;

      } else if (b0 == 1) {
        this->strongHot = true;
        this->strongCool = true;

      } else if (b0 == 2) {
        this->strongHot = false;
        this->strongCool = false;

      } else if (b0 == 3) {
        this->strongHot = true;
        this->strongCool = false;
      }
      break;

    case Feature::LED_LIGHT:
      this->lightType = b0;
      break;

    case Feature::TEMP_RANGES:
      this->cool_adjust_down_temp = b0 / 2;
      this->cool_adjust_up_temp = reader[1] / 2;
      this->auto_adjust_down_temp = reader[2] / 2;
      this->auto_adjust_up_temp = reader[3] / 2;
      this->hot_adjust_down_temp = reader[4] / 2;
      this->hot_adjust_up_temp = reader[5] / 2;
      this->isHavePoint = (reader.size() > 6) ? reader[6] : reader[2];
      break;

    case Feature::IS_TWINS:
      this->isTwins = b0 == 1;
      break;

    case Feature::ECO_MODES:
      this->eco = b0 == 1;
      this->special_eco = b0 == 2;
      break;

    case Feature::EIGHT_HEAT:
      this->eightHot = b0 == 1;
      break;

    case Feature::MODES:
      this->hotcold = b0;

      if (b0 == CmdB5::COLD_HOT) {
        this->cool = true;
        this->hot = true;
        this->dry = true;
        this->auto_ = true;

      } else if (b0 == CmdB5::HOT) {
        this->cool = false;
        this->dry = false;
        this->hot = true;
        this->auto_ = true;

      } else if (b0 == CmdB5::COLD_SUB) {
        this->cool = true;
        this->dry = false;
        this->hot = false;
        this->auto_ = false;

      } else if (b0 == CmdB5::COLD_SUB_COLD_HOT) {
        this->cool = true;
        this->dry = false;
        this->hot = true;
        this->auto_ = false;
        this->wind = true;

      } else if (b0 == CmdB5::COLD_SUB_COLD) {
        this->cool = true;
        this->dry = true;
        this->hot = false;
        this->auto_ = false;
        this->wind = true;

      } else {
        // CmdB5::COLD
        this->hot = false;
        this->cool = true;
        this->dry = true;
        this->auto_ = true;
      }
      break;

    case Feature::SWING_MODES:
      if (b0 == 0) {
        this->leftrightFan = false;
        this->updownFan = true;

      } else if (b0 == 1) {
        this->leftrightFan = true;
        this->updownFan = true;

      } else if (b0 == 2) {
        this->leftrightFan = false;
        this->updownFan = false;

      } else if (b0 == 3) {
        this->leftrightFan = true;
        this->updownFan = false;
      }
      break;

    case Feature::POWER_FUNC:
      if (b0 < 2) {
        this->powerCal = false;
        this->powerCalSetting = false;

      } else if (b0 == 2) {
        this->powerCal = true;
        this->powerCalSetting = false;

      } else if (b0 == 3) {
        this->powerCal = true;
        this->powerCalSetting = true;
      }
      break;

    case Feature::AIR_FILTER:
      if (b0 == 0) {
        this->nestCheck = false;
        this->nestNeedChange = false;

      } else if (b0 < 3) {
        this->nestCheck = true;
        this->nestNeedChange = false;

      } else if (b0 == 3) {
        this->nestCheck = false;
        this->nestNeedChange = true;

      } else if (b0 == 4) {
        this->nestCheck = true;
        this->nestNeedChange = true;
      }
      break;

    case Feature::IS_FOUR_DIRECTION:
      this->isFourDirection = b0 == 1;
      break;
  }
}

uint8_t CmdB5::read(const PropertiesData &data) {
  auto b5 = data.getReader();

  for (; b5.available() > 3; b5.advance())
    m_setFuncEnable(b5);

  return (b5.available() == 3) ? b5[-3] : 0;
}

#define LOG_CAPABILITY(str, condition) \
  if (condition) \
    LOG_CONFIG(TAG, str);

void CmdB5::dump() const {
  LOG_CONFIG(TAG, "Midea Capabilities Report:");

  if (this->auto_) {
    LOG_CONFIG(TAG, "  [x] Auto Mode");
    LOG_CONFIG(TAG, "      - Min: %d", this->auto_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->auto_adjust_up_temp);
  }

  if (this->cool) {
    LOG_CONFIG(TAG, "  [x] Cool Mode");
    LOG_CONFIG(TAG, "      - Min: %d", this->cool_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->cool_adjust_up_temp);
    LOG_CAPABILITY("      - Turbo", this->strongCool);
  }

  if (this->hot) {
    LOG_CONFIG(TAG, "  [x] Heat Mode");
    LOG_CONFIG(TAG, "      - Min: %d", this->hot_adjust_down_temp);
    LOG_CONFIG(TAG, "      - Max: %d", this->hot_adjust_up_temp);
    LOG_CAPABILITY("      - Turbo", this->strongHot);
  }

  if (this->dry) {
    LOG_CONFIG(TAG, "  [x] Dry Mode");
    //
    LOG_CAPABILITY("      - Auto", this->hasAutoClearHumidity);
    // MODE_DRY_SMART supported. .humidity in command is setpoint
    LOG_CAPABILITY("      - Smart", this->hasHandClearHumidity);
  }

  LOG_CAPABILITY("  [x] Fan Mode", this->wind);
  LOG_CONFIG(TAG, "  [x] Wind Speed: %d", this->hasWindSpeed);
  LOG_CAPABILITY("  [x] Indoor Humidity", this->hasIndoorHumidity);  // Indoor humidity in B1 response
  LOG_CAPABILITY("  [x] Decimal Point", this->isHavePoint);
  LOG_CAPABILITY("  [x] Unit Changeable", this->unitChangeable);
  LOG_CAPABILITY("  [x] Breezeless", this->hasBreeze);  // Only in `COOL` mode. Valid values are: 1, 2, 3, 4.
  LOG_CAPABILITY("  [x] Vertical Direction", this->hasVerticalWind);
  LOG_CAPABILITY("  [x] Horizontal Direction", this->hasHorizontalWind);
  LOG_CAPABILITY("  [x] Twins", this->isTwins);
  LOG_CAPABILITY("  [x] Four Direction", this->isFourDirection);
  LOG_CAPABILITY("  [x] Vertical Swing", this->leftrightFan);
  LOG_CAPABILITY("  [x] Horizontal Swing", this->updownFan);
  LOG_CAPABILITY("  [x] Silky Cool", this->hasNoWindFeel);         // Only in `COOL` mode. Temperature: >= 24°C.
  LOG_CAPABILITY("  [x] ECO", this->eco);                          // Only in `COOL` mode. Temperature: >= 24°C.
  LOG_CAPABILITY("  [x] Special ECO", this->special_eco);          // Only in `AUTO`, `COOL`, `DRY` modes.
  LOG_CAPABILITY("  [x] ECO Intelligent Eye", this->hasSmartEye);  // Not supported modes: `DRY`, `FAN_ONLY`.
  LOG_CAPABILITY("  [x] 8°C Heat", this->hot && this->eightHot);   // Only in `HEAT` mode and not supported by `SLEEP`.

  // in HEAT mode DeviceStatus.ptcAssis must be 1
  LOG_CAPABILITY("  [x] Electric Auxiliary Heat", this->dianfure);
  LOG_CAPABILITY("  [x] Wind ON me", this->hasBlowingPeople);
  LOG_CAPABILITY("  [x] Wind OFF me", this->hasAvoidPeople);
  LOG_CAPABILITY("  [x] LED", this->lightType);
  LOG_CAPABILITY("  [x] Buzzer", this->hasBuzzer);
  LOG_CAPABILITY("  [x] Active Clean", this->hasSelfClean);
  LOG_CAPABILITY("  [x] Breeze Away", this->hasOneKeyNoWindOnMe);  // Only in `AUTO`, `HEAT` modes.

  // DeviceStatus.dusFull. cleanFanTime == 1 in command clear timer
  LOG_CAPABILITY("  [x] Air Filter Cleaning Reminder", this->nestCheck);
  LOG_CAPABILITY("  [x] Air Filter Replacement Reminder", this->nestNeedChange);
  LOG_CAPABILITY("  [x] Power Report", this->powerCal);
  LOG_CAPABILITY("  [x] Power Limit", this->powerCalSetting);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
