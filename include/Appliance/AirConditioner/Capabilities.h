#pragma once
#include <Arduino.h>
#include <set>

namespace dudanov {
namespace midea {

class FrameData;

namespace ac {

typedef unsigned char uint8_t;

class CmdB5 {
 public:
  // Read AC capabilities from frame data
  uint8_t read(const FrameData &data);
  // Dump capabilities
  void dump() const;
  // Checking for need sending B1 query
  bool isNeedB1Query() const;

  CmdB5 &setBaseFunc();
  CmdB5 &toSubCool();
  CmdB5 &toOnlyCool();
  CmdB5 &toOnlyHot();
  CmdB5 &toAllEnable();

  enum ACType : uint8_t {
    COLD,
    COLD_HOT,
    HOT,
    COLD_SUB,
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
  bool hasHorizontalWind{false};
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
  uint8_t cool_adjust_up_temp{30};
  uint8_t cool_adjust_down_temp{17};
  uint8_t auto_adjust_up_temp{30};
  uint8_t auto_adjust_down_temp{17};
  uint8_t hot_adjust_up_temp{30};
  uint8_t hot_adjust_down_temp{17};
  uint8_t lightType{0};
  uint8_t fresh_air_switch{0};
  uint8_t fresh_air_speed{0};
  uint8_t fresh_air_temp{0};
  uint8_t fresh_air_mode{0};
};

class DeviceStatus {
 public:
  /// 8 degree heating
  uint8_t Eight_Hot;
  int atwDhwSetTemperature;
  uint8_t avoidPeople;
  uint8_t blowingPeople;
  /// Clear Dust Full
  uint8_t cleanFanTime;
  /// Dust Full
  uint8_t dusFull;
  /// ECO
  uint8_t eco;
  uint8_t errInfo;
  /// windspeed
  uint8_t fanSpeed;
  /// No Wind sense
  uint8_t hasNoWindFeel;
  /// Humidity
  uint8_t humidity;
  /// indoor
  double indoor_temp;
  /// left and right wind
  uint8_t leftRightFan;
  uint8_t light;
  /// mode
  uint8_t mode;
  uint8_t noWindOnMe;
  /// outdoor
  double outdoor_temp;
  /// Equipment status: Power on/off
  uint8_t powerStatus;
  uint8_t protocol;
  uint8_t ptcAssis;
  /// 39 Self-Clean
  uint8_t selfClean;
  /// 18-Temperature
  int setNewTemperature;
  /// Set temperature
  float setTemperature;
  /// Whether the temperature includes decimal point
  uint8_t setTemperature_dot;
  /// SleepFunc
  uint8_t sleepFunc;
  /// TempUnit
  uint8_t tempUnit;
  uint8_t timer_off;
  /// Timer off/hour
  uint8_t timer_off_hour;
  /// Timer off/minute
  uint8_t timer_off_min;
  uint8_t timer_on;
  /// Timer On/Hour
  uint8_t timer_on_hour;
  /// Timer on/minute
  uint8_t timer_on_min;
  /// Strong
  uint8_t tubro;
  /// Upwind
  uint8_t updownFan;

 protected:
  uint8_t T1_dot;
  uint8_t T4_dot;
  uint8_t addupCmd;
  uint8_t alarmSleep;
  uint8_t catchCold;
  uint8_t changeCosySleep;
  uint8_t childSleepMode;
  uint8_t cleanUp;
  uint8_t clearCmd;
  uint8_t controlSrc;
  uint8_t coolFan;
  uint8_t cosySleep;
  uint8_t double_temp;
  uint8_t dryClean;
  uint8_t errMark;
  uint8_t exchangeAir;
  int expand_temp;
  uint8_t feelOwn;
  uint8_t imodeResume;
  uint8_t keyBtn;
  uint8_t keyStatus;
  uint8_t lowFerqFan;
  uint8_t naturalFan;
  uint8_t nightLight;
  uint8_t peakElec;
  uint8_t pmvFunc_H;
  uint8_t pmvFunc_L;
  uint8_t powerSaver;
  uint8_t powerValue1;
  uint8_t powerValue2;
  uint8_t powerValue3;
  uint8_t ptcButton;
  uint8_t pwmMode;
  uint8_t resetCmd;
  uint8_t runmodeCmd;
  uint8_t save;
  uint8_t selfFeelOwn;
  uint8_t setCmd;
  uint8_t setExpand;
  uint8_t setExpand_dot;
  uint8_t setallusageCmd;
  uint8_t superFan;
  uint8_t test2;
  uint8_t time1;
  uint8_t time3;
  uint8_t timeLimit;
  uint8_t timerEffe;
  uint8_t timerMode;
  uint8_t wiseEye;
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
