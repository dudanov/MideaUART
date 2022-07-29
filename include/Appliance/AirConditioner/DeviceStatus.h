#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

static bool inRange(float value) { return value > 16.0F && value < 30.0F; }

static float getTemp(int integer, int decimal, bool fahrenheits) {
  integer -= 50;
  if (!fahrenheits && decimal > 0)
    return static_cast<float>(integer / 2) + static_cast<float>(decimal) * ((integer >= 0) ? 0.1F : -0.1F);
  if (decimal >= 5)
    return static_cast<float>(integer / 2) + ((integer >= 0) ? 0.5F : -0.5F);
  return static_cast<float>(integer) * 0.5F;
}
typedef unsigned char uint8_t;

class DeviceStatus {
 public:
  DeviceStatus(const DeviceStatus &deviceStatus) {
    this->powerStatus = deviceStatus.powerStatus;
    this->setTemperature = deviceStatus.setTemperature;
    this->setTemperature_dot = deviceStatus.setTemperature_dot;
    this->mode = deviceStatus.mode;
    this->fanSpeed = deviceStatus.fanSpeed;
    this->timer_on_hour = deviceStatus.timer_on_hour;
    this->timer_on = deviceStatus.timer_on;
    this->timer_off_hour = deviceStatus.timer_off_hour;
    this->timer_off = deviceStatus.timer_off;
    this->timer_off_min = deviceStatus.timer_off_min;
    this->timer_on_min = deviceStatus.timer_on_min;
    this->updownFan = deviceStatus.updownFan;
    this->leftRightFan = deviceStatus.leftRightFan;
    this->eco = deviceStatus.eco;
    this->tempUnit = deviceStatus.tempUnit;
    this->tubro = deviceStatus.tubro;
    this->cleanFanTime = deviceStatus.cleanFanTime;
    this->dusFull = deviceStatus.dusFull;
    this->Eight_Hot = deviceStatus.Eight_Hot;
    this->indoor_temp = deviceStatus.indoor_temp;
    this->outdoor_temp = deviceStatus.outdoor_temp;
    this->sleepFunc = deviceStatus.sleepFunc;
    this->catchCold = deviceStatus.catchCold;
    this->humidity = deviceStatus.humidity;
    this->setNewTemperature = deviceStatus.setNewTemperature;
    this->hasNoWindFeel = deviceStatus.hasNoWindFeel;
    this->selfClean = deviceStatus.selfClean;
    this->noWindOnMe = deviceStatus.noWindOnMe;
    this->blowingPeople = deviceStatus.blowingPeople;
    this->avoidPeople = deviceStatus.avoidPeople;
  }
  static DeviceStatus fromA0Response(const FrameData &data) {
    DeviceStatus s;
    s.powerStatus = data.m_getValue(1, 1);
    s.setTemperature = ((data[1] & 62) >> 1) + 12;
    s.setTemperature_dot = (data[1] & 64) >> 6;
    s.errMark = (data[1] & 128) >> 7;
    s.mode = (data[2] & 224) >> 5;
    s.fanSpeed = data[3] & 127;
    s.timer_on_hour = (data[4] & 124) >> 2;
    s.timer_on_min = (data[4] & 3) | ((data[6] & 240) >> 4);
    s.timer_on = (data[4] & 128) >> 7;
    s.timer_off_hour = (data[5] & 124) >> 2;
    s.timer_off_min = (data[5] & 3) | (data[6] & 15);
    s.timer_off = (data[5] & 128) >> 7;
    if (!s.timer_off) {
      s.timer_off_min = 0;
      s.timer_off_hour = 0;
    }
    if (!s.timer_on) {
      s.timer_on_min = 0;
      s.timer_on_hour = 0;
    }
    s.updownFan = (data[7] & 12) ? 1 : 0;
    s.leftRightFan = (data[7] & 3) ? 1 : 0;
    s.cosySleep = data[8] & 3;
    s.save = (data[8] & 8) >> 3;
    s.lowFerqFan = (data[8] & 16) >> 4;
    s.tubro = (data[8] & 32) >> 5;
    s.feelOwn = (data[8] & 128) >> 7;
    s.exchangeAir = (data[9] & 2) >> 1;
    s.dryClean = (data[9] & 4) >> 2;
    s.ptcAssis = (data[9] & 8) >> 3;
    s.eco = (data[9] & 16) >> 4;
    s.cleanUp = (data[9] & 32) >> 5;
    s.tempUnit = (data[9] & 128) >> 7;
    s.sleepFunc = data[10] & 1;
    if (!s.tubro)
      s.tubro = (data[10] & 2) >> 1;
    s.catchCold = (data[10] & 8) >> 3;
    s.nightLight = (data[10] & 16) >> 4;
    s.peakElec = (data[10] & 32) >> 5;
    s.naturalFan = (data[10] & 64) >> 6;
    s.light = data[11] & 7;
    s.pwmMode = data[11] & 15;
    s.setExpand_dot = data[12] & 1;
    s.setExpand = (data[12] & 31) + 12;
    s.double_temp = (data[12] & 64) >> 6;
    s.Eight_Hot = (data[12] & 128) >> 7;
    s.humidity = data[13] & 127;
    s.hasNoWindFeel = (data[14] & 8) >> 3;
    if (s.tempUnit) {
    }
  }
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

    this->indoor_temp = getTemp(data[11], data[15] % 16, data[10] & 4);
    this->outdoor_temp = getTemp(data[12], data[15] / 16, data[10] & 4);

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

  /// 8 degrees heating
  uint8_t Eight_Hot;
  int atwDhwSetTemperature;
  uint8_t avoidPeople;
  uint8_t blowingPeople;
  /// clear dust
  uint8_t cleanFanTime;
  /// full of dust
  uint8_t dusFull;
  /// ECO
  uint8_t eco;
  uint8_t errInfo;
  /// wind speed
  uint8_t fanSpeed;
  /// no wind
  uint8_t hasNoWindFeel;
  /// humidity
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
  /// Device Status: On/Off
  uint8_t powerStatus;
  /// Protocol version
  uint8_t protocol;
  /// Electric auxiliary heater
  uint8_t ptcAssis;
  /// 39 Self-cleaning
  uint8_t selfClean;
  /// 18-Temperature
  int setNewTemperature;
  /// set temperature
  float setTemperature;
  /// Whether the temperature has a decimal point (only for Celsius)
  uint8_t setTemperature_dot;
  /// sleep function
  uint8_t sleepFunc;
  /// temperature unit
  uint8_t tempUnit;
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
  uint8_t tubro;
  /// up and down
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
