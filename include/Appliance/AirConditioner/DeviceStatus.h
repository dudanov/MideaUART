#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

class DeviceStatus {
  double outdoor_temp;
  double indoor_temp;
  int atwDhwSetTemperature;
  int setNewTemperature;
  float setTemperature;
  uint8_t Eight_Hot;
  uint8_t avoidPeople;
  uint8_t blowingPeople;
  uint8_t cleanFanTime;
  uint8_t dusFull;
  uint8_t eco;
  uint8_t errInfo;
  uint8_t fanSpeed;
  uint8_t hasNoWindFeel;
  uint8_t humidity;
  uint8_t leftRightFan;
  uint8_t light;
  uint8_t mode;
  uint8_t noWindOnMe;
  uint8_t powerStatus;
  uint8_t protocol;
  uint8_t ptcAssis;
  uint8_t selfClean;
  uint8_t setTemperature_dot;
  uint8_t sleepFunc;
  uint8_t tempUnit;
  uint8_t timer_off;
  uint8_t timer_off_hour;
  uint8_t timer_off_min;
  uint8_t timer_on;
  uint8_t timer_on_hour;
  uint8_t timer_on_min;
  uint8_t tubro;
  uint8_t updownFan;

 protected:
  int expand_temp;
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
