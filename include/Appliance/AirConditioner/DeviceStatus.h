#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

class DeviceStatus {
 public:
  DeviceStatus() = default;
  DeviceStatus(const DeviceStatus &deviceStatus);
  /// 8 degrees heating
  bool Eight_Hot;
  // int atwDhwSetTemperature;
  bool avoidPeople;
  bool blowingPeople;
  /// clear dust
  bool cleanFanTime;
  /// full of dust
  bool dusFull;
  /// ECO
  bool eco;
  uint8_t errInfo;
  /// wind speed
  uint8_t fanSpeed{102};
  // "Silky Cool" preset
  bool hasNoWindFeel;
  /// humidity
  uint8_t humidity;
  /// indoor
  float indoor_temp;
  /// left and right wind
  bool leftRightFan;
  uint8_t light;
  /// mode
  uint8_t mode{2};
  bool noWindOnMe;
  /// outdoor
  float outdoor_temp;
  /// Device Status: On/Off
  bool powerStatus{true};
  /// Protocol version
  uint8_t protocol;
  /// Electric auxiliary heater
  bool ptcAssis;
  /// 39 Self-cleaning
  bool selfClean;
  /// 18-Temperature
  int setNewTemperature;
  /// set temperature
  uint8_t setTemperature{26};
  /// Whether the temperature has a decimal point (only for Celsius)
  bool setTemperature_dot;
  /// sleep function
  bool sleepFunc;
  /// temperature unit
  bool tempUnit;
  bool timer_off;
  /// time off/hour
  uint8_t timer_off_hour;
  /// time off/min
  uint8_t timer_off_min;
  bool timer_on;
  /// timed start/hour
  uint8_t timer_on_hour;
  /// Timing start/minute
  uint8_t timer_on_min;
  /// strong
  bool turbo;
  /// up and down
  bool updownFan;

  void convertUnits();
  uint8_t T1_dot;
  uint8_t T4_dot;
  uint8_t addupCmd;
  bool alarmSleep;
  bool catchCold;
  bool changeCosySleep;
  bool childSleepMode;
  bool cleanUp;
  uint8_t clearCmd;
  uint8_t controlSrc;
  bool coolFan;
  uint8_t cosySleep;
  bool double_temp;
  bool dryClean;
  bool errMark;
  bool exchangeAir;
  int expand_temp;
  bool feelOwn;
  bool imodeResume;
  uint8_t keyBtn;
  uint8_t keyStatus;
  bool lowFerqFan;
  bool naturalFan;
  bool nightLight;
  bool peakElec;
  uint8_t pmvFunc_H;
  uint8_t pmvFunc_L;
  bool powerSaver;
  uint8_t powerValue1;
  uint8_t powerValue2;
  uint8_t powerValue3;
  bool ptcButton;
  uint8_t pwmMode;
  uint8_t resetCmd;
  uint8_t runmodeCmd;
  bool save;
  bool selfFeelOwn;
  uint8_t setCmd;
  uint8_t setExpand;
  bool setExpand_dot;
  uint8_t setallusageCmd;
  uint8_t superFan;
  bool test2;
  uint8_t time1;
  uint8_t time3;
  uint8_t timeLimit;
  uint8_t timerEffe;
  bool timerMode;
  bool wiseEye;
};

struct B1Status {
  uint8_t hWindDirection{};
  uint8_t vWindDirection{};
  uint8_t indoorHumidity{};
  std::vector<uint8_t> mMasterValues{};
  std::vector<uint8_t> mSlaveValues{};
  bool isSelfCleanOn{};
  uint8_t breezeType{1};
  bool isBuzzerOn{true};
  bool isNoWindFeelOn{false};
  bool isSoundOn{false};
  bool isBlowingPeopleOn{false};
  bool isAvoidPeopleOn{false};
  bool isOneKeyNoWindOnMeOn{false};
  bool isCombineBreezeOn{false};
  bool isSmartEyeOn{false};
  bool DehumLight{false};
};

class FrameStatusData : public FrameData {
 public:
  DeviceStatus updateFromA0();
  DeviceStatus updateFromA1();
  DeviceStatus updateFromC0();
  void to40Command(const DeviceStatus &status);
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
