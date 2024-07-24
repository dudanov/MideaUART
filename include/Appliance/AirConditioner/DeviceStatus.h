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

  float getTargetTemperature() const;
  void setTargetTemperature(float value);

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
  /// Electric auxiliary heater
  bool ptcAssis;
  /// 39 Self-cleaning
  bool selfClean;
  /// set temperature
  uint8_t setTemperature{26};
  /// Whether the temperature has a decimal point (only for Celsius)
  bool setTemperature_dot;
  /// sleep function
  bool sleepFunc;
  /// temperature unit
  bool tempUnit;
  /// On Timer (minutes)
  uint16_t timer_on;
  /// Off Timer (minutes)
  uint16_t timer_off;
  /// strong
  bool turbo;
  /// up and down
  bool updownFan;

  bool alarmSleep;
  bool catchCold;
  bool changeCosySleep;
  bool childSleepMode;
  bool cleanUp;
  bool coolFan;
  uint8_t cosySleep;
  bool double_temp;
  bool dryClean;
  bool errMark;
  bool exchangeAir;
  bool feelOwn;
  bool imodeResume;
  bool lowFreqFan;
  bool naturalFan;
  bool nightLight;
  bool peakElec;
  bool powerSaver;
  bool ptcButton;
  uint8_t pwmMode;
  bool save;
  bool selfFeelOwn;
  uint8_t setExpand;
  bool setExpand_dot;
  bool test2;
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
  void updateFromA0(DeviceStatus &s);
  void updateFromA1(DeviceStatus &s);
  void updateFromC0(DeviceStatus &s);
  void to40Command(const DeviceStatus &s, bool beeper);
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
