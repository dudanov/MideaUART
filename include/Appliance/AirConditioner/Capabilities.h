#pragma once
#include <Arduino.h>
#include <set>

namespace dudanov {
namespace midea {

class FrameData;

namespace ac {

typedef unsigned char uint8_t;

class Capabilities {
 public:
  // Read from frames
  bool read(const FrameData &data);
  // Dump capabilities
  void dump() const;

  bool hasNoWindSpeed;
  // uint8_t hasWindSpeed;
  uint8_t hotcold;
  bool updownFan{false};
  bool unitChangeable{true};
  bool eco{false};
  bool special_eco{false};
  bool leftrightFan{true};
  bool eightHot{false};
  bool cool{true};
  bool hot{true};
  bool dry{true};
  bool auto1{true};
  bool wind{true};
  bool mutilTemp{true};
  bool powerCal{false};
  bool powerCalSetting{false};
  bool powerCalBCD{false};
  bool selfcheck{true};
  bool nestCheck{false};
  bool nestNeedChange{false};
  bool dianfure{false};
  bool strongCool{true};
  bool strongHot{false};
  bool hasNoWindFeel{false};
  bool hasAutoClearHumidity{false};
  bool hasHandClearHumidity{false};
  uint8_t cool_adjust_up_temp{30};
  uint8_t cool_adjust_down_temp{17};
  uint8_t auto_adjust_up_temp{30};
  uint8_t auto_adjust_down_temp{17};
  uint8_t hot_adjust_up_temp{30};
  uint8_t hot_adjust_down_temp{17};
  bool isHavePoint{false};
  uint8_t zNum{0};
  uint8_t leftNum{0};
  bool hasBlowingPeople{false};
  bool hasAvoidPeople{false};
  bool hasSelfClean{false};
  bool hasOneKeyNoWindOnMe{false};
  bool hasBreeze{false};
  bool hasSmartEye{false};
  bool hasIndoorHumidity{false};
  uint8_t lightType{0};
  bool hasBuzzer{false};
  bool hasHorizontalWind{false};
  bool hasVerticalWind{false};
  bool isTwins{false};
  bool isFourDirection{false};
  bool hasJetCool{false};
  bool hasFreshAir{false};
  bool isJetCoolEnable{false};
  bool isFreshAirEnable{false};
  uint8_t fresh_air_switch{0};
  uint8_t fresh_air_speed{0};
  uint8_t fresh_air_temp{0};
  uint8_t fresh_air_mode{0};

 private:
  bool power{true};
  bool mode{true};
  bool temperature{true};
  bool windSpeed{true};
  bool airCheck{false};
  bool indoor_temperature{false};
  bool turbo{false};
  bool outdoor_temperature{false};
  bool dishui{false};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
