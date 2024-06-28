#include "Appliance/AirConditioner/DeviceStatus.h"

namespace dudanov {
namespace midea {
namespace ac {

static int celsius_to_fahrenheits(float value) { return static_cast<int>(value * 1.8F) + 32; }
static float fahrenheits_to_celsius(int value) { return static_cast<float>(value - 32) / 1.8F; }

static float get_temperature(int integer, int decimal, bool fahrenheits) {
  integer -= 50;
  if (!fahrenheits && decimal > 0)
    return static_cast<float>(integer / 2) + static_cast<float>(decimal) * ((integer >= 0) ? 0.1F : -0.1F);
  if (decimal >= 5)
    return static_cast<float>(integer / 2) + ((integer >= 0) ? 0.5F : -0.5F);
  return static_cast<float>(integer) * 0.5F;
}

DeviceStatus::DeviceStatus(const DeviceStatus &deviceStatus) {
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

void DeviceStatus::updateFromA0(const FrameData &data) {
  this->powerStatus = data[1] & 1;
  this->setTemperature = static_cast<float>(((data[1] >> 1) & 31) + 12);
  this->setTemperature_dot = data[1] & 64;
  this->errMark = data[1] & 128;
  this->mode = data[2] >> 5;
  this->fanSpeed = data[3] & 127;
  this->timer_on_hour = (data[4] >> 2) & 31;
  this->timer_on_min = (data[4] & 3) | (data[6] >> 4);
  this->timer_on = data[4] & 128;
  this->timer_off_hour = (data[5] >> 2) & 31;
  this->timer_off_min = (data[5] & 3) | (data[6] & 15);
  this->timer_off = data[5] & 128;
  if (!this->timer_off) {
    this->timer_off_min = 0;
    this->timer_off_hour = 0;
  }
  if (!this->timer_on) {
    this->timer_on_min = 0;
    this->timer_on_hour = 0;
  }
  this->updownFan = data[7] & 12;
  this->leftRightFan = data[7] & 3;
  this->cosySleep = data[8] & 3;
  this->save = data[8] & 8;
  this->lowFerqFan = data[8] & 16;
  this->tubro = data[8] & 32;
  this->feelOwn = data[8] & 128;
  this->exchangeAir = data[9] & 2;
  this->dryClean = data[9] & 4;
  this->ptcAssis = data[9] & 8;
  this->eco = data[9] & 16;
  this->cleanUp = data[9] & 32;
  this->tempUnit = data[9] & 128;
  this->sleepFunc = data[10] & 1;
  if (!this->tubro)
    this->tubro = data[10] & 2;
  this->catchCold = data[10] & 8;
  this->nightLight = data[10] & 16;
  this->peakElec = data[10] & 32;
  this->naturalFan = data[10] & 64;
  this->pwmMode = data[11] & 15;
  this->light = (data[11] >> 4) & 7;
  this->setExpand_dot = data[12] & 1;
  this->setExpand = (data[12] & 31) + 12;
  this->double_temp = data[12] & 64;
  this->Eight_Hot = data[12] & 128;
  this->humidity = data[13] & 127;
  this->hasNoWindFeel = data[14] & 8;
  if (this->tempUnit)
    this->convertUnits();
}

void DeviceStatus::updateFromA1(const FrameData &data) {
  this->indoor_temp = static_cast<float>(data[13] - 50) * 0.5F;
  this->outdoor_temp = static_cast<float>(data[14] - 50) * 0.5F;
  this->humidity = data[17] & 127;
}

void DeviceStatus::updateFromC0(const FrameData &data) {
  this->powerStatus = data[1] & 1;
  this->imodeResume = data[1] & 4;
  this->timerMode = data[1] & 16;
  this->test2 = data[1] & 32;
  this->errMark = data[1] & 128;
  this->setTemperature = static_cast<float>(data[2] & 15 + 16);
  this->setTemperature_dot = data[2] & 16;
  this->mode = data[2] >> 5;
  this->fanSpeed = data[3] & 127;
  if (this->fanSpeed == 101)
    this->fanSpeed = 102;
  this->timer_on_hour = (data[4] >> 2) & 31;
  this->timer_on_min = (data[4] & 3) | (data[6] >> 4);
  this->timer_on = data[4] & 128;
  this->timer_off_hour = (data[5] & 124) >> 2;
  this->timer_off_min = (data[5] & 3) | (data[6] & 15);
  this->timer_off = data[5] & 128;
  if (!this->timer_off) {
    this->timer_off_min = 0;
    this->timer_off_hour = 0;
  }
  if (!this->timer_on) {
    this->timer_on_min = 0;
    this->timer_on_hour = 0;
  }
  this->updownFan = data[7] & 12;
  this->leftRightFan = data[7] & 3;
  this->cosySleep = data[8] & 3;
  this->save = data[8] & 8;
  this->lowFerqFan = data[8] & 16;
  this->tubro = data[8] & 32;
  this->feelOwn = data[8] & 128;
  this->childSleepMode = data[9] & 1;
  this->naturalFan = data[9] & 2;
  this->dryClean = data[9] & 4;
  this->ptcAssis = data[9] & 8;
  this->eco = data[9] & 16;
  this->cleanUp = data[9] & 32;
  this->sleepFunc = data[9] & 64;
  this->selfFeelOwn = data[9] & 128;
  this->sleepFunc = data[10] & 1;
  if (!this->tubro)
    this->tubro = data[10] & 2;
  this->tempUnit = data[10] & 4;
  this->exchangeAir = data[10] & 8;
  this->nightLight = data[10] & 16;
  this->catchCold = data[10] & 32;
  this->peakElec = data[10] & 64;
  this->coolFan = data[10] & 128;
  this->Eight_Hot = data[12] & 128;
  this->dusFull = data[13] & 32;
  this->pwmMode = data[14] & 15;
  this->light = (data[14] >> 4) & 7;
  this->T1_dot = data[15] & 15;
  this->T4_dot = data[15] >> 4;
  this->indoor_temp = get_temperature(data[11], this->T1_dot, this->tempUnit);
  this->outdoor_temp = get_temperature(data[12], this->T4_dot, this->tempUnit);
  this->errInfo = data[16];
  if (data[13] & 31) {
    this->setNewTemperature = static_cast<float>((data[13] & 31) + 12);
    this->setTemperature = this->setNewTemperature;
  }
  this->humidity = data[19] & 127;
  if (data.size() >= 23)
    this->hasNoWindFeel = data[22] & 8;
  if (data.size() >= 24) {
    this->double_temp = data[21] & 64;
    this->Eight_Hot = data[21] & 128;
  } else {
    this->double_temp = 0;
    this->Eight_Hot = 0;
  }
  if (this->tempUnit)
    this->convertUnits();
}

FrameData DeviceStatus::to40Command() const {
  FrameData data{{0x40, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

  uint8_t lfanSpeed = this->fanSpeed;
  bool lturbo = this->tubro;
  bool leco = this->eco;

  if (!this->powerStatus) {
    leco = false;
    lturbo = false;
  }

  if (this->mode == 5) {
    leco = false;
    lturbo = false;
  }

  if (this->mode != 3 && lfanSpeed == 101)
    lfanSpeed = 102;

  data[1] = 64 | this->test2 * 32 | this->timerMode * 16 | this->childSleepMode * 8 | this->imodeResume * 4 | 2 |
            this->powerStatus * 1;

  int lsetTemp = static_cast<int>(this->setTemperature);
  bool lsetTemp_dot = this->setTemperature_dot;

  if (lsetTemp >= 50) {
    int tmp = static_cast<int>(fahrenheits_to_celsius(lsetTemp) * 2.0F + 0.5F);
    lsetTemp = tmp / 2;
    lsetTemp_dot = tmp % 2;
  }

  int lsetTempNew = (lsetTemp - 12) % 32;
  lsetTemp -= 16;
  if (lsetTemp < 1 || lsetTemp > 14)
    lsetTemp = 1;

  data[2] = this->mode * 32 | lsetTemp_dot * 16 | lsetTemp * 1;
  data[3] = this->timerEffe * 128 | lfanSpeed % 128 * 1;

  /* Setting timers. Initialized off. Therefore, we process only if enabled. */

  if (this->timer_on) {
    data[4] = 128 | this->timer_on_hour * 4 | this->timer_on_min / 15;
    data[6] = this->timer_on_min % 15 * 16;
  }

  if (this->timer_off) {
    data[5] = 128 | this->timer_off_hour * 4 | this->timer_off_min / 15;
    data[6] |= this->timer_off_min % 15;
  }

  data[7] = 48 | this->updownFan * 12 | this->leftRightFan * 3;
  data[8] = this->feelOwn * 128 | this->powerSaver * 64 | lturbo * 32 | this->lowFerqFan * 16 | this->save * 8 |
            this->alarmSleep * 4 | this->cosySleep % 4 * 1;
  data[9] = leco * 128 | this->changeCosySleep * 64 | this->cleanUp * 32 | this->ptcButton * 16 | this->ptcAssis * 8 |
            this->dryClean * 4 | this->exchangeAir * 2 | this->wiseEye * 1;
  data[10] = this->cleanFanTime * 128 | this->dusFull * 64 | this->peakElec * 32 | this->nightLight * 16 |
             this->catchCold * 8 | this->tempUnit * 4 | lturbo * 2 | this->sleepFunc * 1;
  data[15] = this->naturalFan * 64;
  data[18] = lsetTempNew;
  data[19] = this->humidity;
  data[21] = this->Eight_Hot * 128 | this->double_temp * 64 | this->setExpand % 32 * 2 | this->setExpand_dot * 1;

  static uint8_t id = 4;
  if (++id < 4 || id >= 255)
    id = 4;
  data[23] = id;

  data.appendCRC();
  return data;
}

void DeviceStatus::convertUnits() {
  this->indoor_temp = celsius_to_fahrenheits(this->indoor_temp);
  this->outdoor_temp = celsius_to_fahrenheits(this->outdoor_temp);
  this->setTemperature = celsius_to_fahrenheits(this->setTemperature + this->setTemperature_dot * 0.5F);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov