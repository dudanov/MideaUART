#pragma once
#include <Arduino.h>
#include <set>

namespace dudanov {
namespace midea {

class FrameData;

namespace ac {

enum Feature : uint16_t {
  VERTICAL_WIND = 0x0009,
  HORIZONTAL_WIND = 0x000A,
  INDOOR_HUMIDITY = 0x0015,
  SILKY_COOL = 0x0018,
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

class CmdB5 {
 public:
  // Read AC capabilities from frame data
  uint8_t read(const FrameData &data);
  // Dump capabilities
  void dump() const;
  // Checking for need sending B1 query
  bool isNeedB1Query() const;

  CmdB5 &setBaseFunc();
  CmdB5 &toOnlyCool();
  CmdB5 &toAllEnable();
  CmdB5 &toOnlyHot();
  CmdB5 &toSubCool();

  enum ACType : uint8_t {
    COLD,      // toOnlyCool()
    COLD_HOT,  // toAllEnable()
    HOT,       // toOnlyHot()
    COLD_SUB,  // toSubCool()
    COLD_SUB_COLD_HOT,
    COLD_SUB_COLD,
  };

  ACType getDeviceType() const { return static_cast<ACType>(this->hotcold); }

  // Fahrenheits
  bool unitChangeable{true};
  // Cool mode
  bool cool{true};
  // Heat mode
  bool hot{true};
  // Dry mode
  bool dry{true};
  // Auto mode
  bool auto1{true};
  // FanOnly mode
  bool wind{true};
  bool mutilTemp{true};
  bool selfcheck{true};
  // Cool Turbo preset
  bool strongCool{true};
  // Vertical swing
  bool leftrightFan{true};
  // Horizontal swing
  bool updownFan{false};
  // ECO preset
  bool eco{false};
  // Special ECO preset
  bool special_eco{false};
  // Frost Protection
  bool eightHot{false};
  bool powerCal{false};
  bool powerCalSetting{false};
  bool powerCalBCD{false};
  // Air Filter
  bool nestCheck{false};
  bool nestNeedChange{false};
  // Electric auxiliary heat
  bool dianfure{false};
  // Heat Turbo preset
  bool strongHot{false};
  bool hasNoWindFeel{false};
  bool hasAutoClearHumidity{false};
  bool hasHandClearHumidity{false};
  bool isHavePoint{false};
  bool hasBlowingPeople{false};
  bool hasAvoidPeople{false};
  // Active clean
  bool hasSelfClean{false};
  bool hasOneKeyNoWindOnMe{false};
  bool hasBreeze{false};
  bool hasSmartEye{false};
  bool hasIndoorHumidity{false};
  // Sound
  bool hasBuzzer{false};
  // adjusting the direction of horizontal air flow
  bool hasHorizontalWind{false};
  // adjusting the direction of vertical air flow
  bool hasVerticalWind{false};
  bool isTwins{false};
  bool isFourDirection{false};
  bool hasJetCool{false};
  bool hasFreshAir{false};
  bool isJetCoolEnable{false};
  bool isFreshAirEnable{false};
  bool hasNoWindSpeed;
  uint8_t hasWindSpeed;
  uint8_t hotcold;
  // Maximum temperature in COOL mode
  uint8_t cool_adjust_up_temp{30};
  // Minimal temperature in COOL mode
  uint8_t cool_adjust_down_temp{17};
  // Maximum temperature in AUTO mode
  uint8_t auto_adjust_up_temp{30};
  // Minimal temperature in AUTO mode
  uint8_t auto_adjust_down_temp{17};
  // Maximum temperature in HEAT mode
  uint8_t hot_adjust_up_temp{30};
  // Minimal temperature in HEAT mode
  uint8_t hot_adjust_down_temp{17};
  uint8_t lightType{0};
  uint8_t fresh_air_switch{0};
  uint8_t fresh_air_speed{0};
  uint8_t fresh_air_temp{0};
  uint8_t fresh_air_mode{0};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
