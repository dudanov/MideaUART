#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

static float s_getTemp(int integer, int decimal, bool fahrenheits) {
  integer -= 50;
  if (!fahrenheits && decimal > 0)
    return static_cast<float>(integer / 2) + static_cast<float>(decimal) * ((integer >= 0) ? 0.1F : -0.1F);
  if (decimal >= 5)
    return static_cast<float>(integer / 2) + ((integer >= 0) ? 0.5F : -0.5F);
  return static_cast<float>(integer) * 0.5F;
}

class DeviceStatus {
  DeviceStatus(const FrameData &data) {
    this->powerStatus = data[1] & 1;
    this->imodeResume = data[1] & 4;
    this->timerMode = (data[1] & 16) >> 1;
    int i = 5;
    this->test2 = data[1] & 32;
    this->errMark = data[1] & 128;
    this->setTemperature = data[2] & 15 + 16;
    this->setTemperature_dot = data[2] & 16;
    this->mode = data[2] >> 5;
    this->fanSpeed = data[3] & 127;
    if (this->fanSpeed == 101)
      this->fanSpeed = 102;
    this->timer_on_hour = (data[4] & 124) >> 2;
    this->timer_on_min = (data[4] & 3) | ((data[6] & 240) >> 4);
    this->timer_on = (data[4] & 128) >> 7;
    this->timer_off_hour = (data[5] & 124) >> 2;
    this->timer_off_min = (data[5] & 3) | (data[6] & 15);
    this->timer_off = (data[5] & 128) >> 7;
    if (this->timer_off == 0) {
      this->timer_off_min = 0;
      this->timer_off_hour = 0;
    }
    if (this->timer_on == 0) {
      this->timer_on_min = 0;
      this->timer_on_hour = 0;
    }
    this->updownFan = (data[7] & 12) ? 1 : 0;
    this->leftRightFan = (data[7] & 3) ? 1 : 0;
    this->cosySleep = data[8] & 3;
    this->save = (data[8] & 8) >> 3;
    this->lowFerqFan = (data[8] & 16) >> 4;
    this->tubro = (data[8] & 32) >> 5;
    this->feelOwn = (data[8] & 128) >> 7;
    this->childSleepMode = data[9] & 1;
    this->naturalFan = (data[9] & 2) >> 1;
    this->dryClean = (data[9] & 4) >> 2;
    this->ptcAssis = (data[9] & 8) >> 3;
    this->eco = (data[9] & 16) >> 4;
    this->cleanUp = (data[9] & 32) >> 5;
    this->sleepFunc = (data[9] & 64) >> 6;
    this->selfFeelOwn = (data[9] & 128) >> 7;
    this->sleepFunc = data[10] & 1;
    if (!this->tubro)
      this->tubro = (data[10] & 2) >> 1;
    this->tempUnit = (data[10] & 4) >> 2;
    this->exchangeAir = (data[10] & 8) >> 3;
    this->nightLight = (data[10] & 16) >> 4;
    this->catchCold = (data[10] & 32) >> 5;
    this->peakElec = (data[10] & 64) >> 6;
    this->coolFan = (data[10] & 128) >> 7;
    this->indoor_temp = (data[11] - 50) / 2;
    this->outdoor_temp = (data[12] - 50) / 2;
    this->Eight_Hot = (data[12] & 128) ? 1 : 0;
    this->dusFull = (data[13] & 32) >> 5;
    this->pwmMode = data[14] & 15;
    this->light = (data[14] & 112) >> 4;
    this->T1_dot = data[15] & 15;
    this->T4_dot = (data[15] & 240) >> 4;

    this->indoor_temp = s_getTemp(data[11], data[15] % 16, data[10] & 4);
    this->outdoor_temp = s_getTemp(data[12], data[15] / 16, data[10] & 4);

    if (this->T1_dot) {
      int i = static_cast<int>(this->indoor_temp);
      if (this->tempUnit == 0) {
        this->indoor_temp = i + this->T1_dot * ((i >= 0) ? 0.1f : -0.1f);
      } else if (this->T1_dot >= 5) {
        this->indoor_temp = i + ((i >= 0) ? 0.5f : -0.5f);
      }
    }
    if (this->tempUnit == 0 && (b = this->T4_dot) > 0) {
      float f3 = b / 10.0f;
      double d11 = this->outdoor_temp >= 0.0d ? ((int) d) + f3 : ((int) d) - f3;
      this->outdoor_temp = d11;
    } else if (this->T4_dot >= 5) {
      double d12 = this->outdoor_temp;
      int i5 = (d12 > 0.0d ? 1 : (d12 == 0.0d ? 0 : -1));
      double d13 = (int) d12;
      double d14 = i5 >= 0 ? d13 + 0.5d : d13 - 0.5d;
      this->outdoor_temp = d14;
    }
    this->errInfo = data[16];
    if ((data[13] & 31) > 0) {
      int i6 = (data[13] & 31) + 12;
      this->setNewTemperature = i6;
      int i7 = this->setNewTemperature;
      this->setTemperature = i7;
    }
    uint8_t b42 = (data[19] & 127);
    this->humidity = b42;
    if (data.length >= 23) {
      uint8_t b43 = ((data[22] & 8) >> 3);
      this->hasNoWindFeel = b43;
    }
    if (data.length >= 24) {
      uint8_t b44 = ((data[21] & 64) >> 6);
      this->double_temp = b44;
      uint8_t b45 = (data[21] & 128) == 0 ? 0 : 1;
      this->Eight_Hot = b45;
    } else {
      this->double_temp = 0;
      this->Eight_Hot = 0;
    }
    if (this->tempUnit == 1) {
      if (this->setTemperature_dot == 0) {
        i = 0;
      }
      ((DecimalFormat) NumberFormat.getInstance(Locale.ENGLISH)).applyPattern("0.0");
      if (inRange(this->indoor_temp)) {
        this->indoor_temp =
            Integer.parseInt(TemperatureUtil.centigrade2Fahrenheit("" + decimalFormat.format(this->indoor_temp)));
      } else if (isSpecialValue(this->indoor_temp)) {
        this->indoor_temp = (int) ((this->indoor_temp * 1.8d) + 32.0d);
      } else {
        this->indoor_temp = (int) new BigDecimal((this->indoor_temp * 1.8d) + 32.0d).setScale(0, 4).floatValue();
      }
      if (inRange(this->outdoor_temp)) {
        this->outdoor_temp =
            Integer.parseInt(TemperatureUtil.centigrade2Fahrenheit("" + decimalFormat.format(this->outdoor_temp)));
      } else if (isSpecialValue(this->outdoor_temp)) {
        this->outdoor_temp = (int) ((this->outdoor_temp * 1.8d) + 32.0d);
      } else {
        this->outdoor_temp = (int) new BigDecimal((this->outdoor_temp * 1.8d) + 32.0d).setScale(0, 4).floatValue();
      }
      L.d("DataResponseOld", "setTemperature  = " + this->setTemperature + " -- dot=" + i);
      StringBuilder sb = new StringBuilder();
      sb.append((int) this->setTemperature);
      sb.append(".");
      sb.append(i);
      this->setTemperature = (float) Integer.parseInt(TemperatureUtil.centigrade2Fahrenheit(sb.toString()));
    }
  }

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
  bool powerStatus;
  uint8_t protocol;
  uint8_t ptcAssis;
  uint8_t selfClean;
  bool setTemperature_dot;
  uint8_t sleepFunc;
  uint8_t tempUnit;
  bool timer_off;
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
  bool errMark;
  uint8_t exchangeAir;
  uint8_t feelOwn;
  bool imodeResume;
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
  bool test2;
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
