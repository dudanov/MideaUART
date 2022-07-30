#include "Appliance/AirConditioner/DeviceStatus.h"

namespace dudanov {
namespace midea {
namespace ac {

static int celsius_to_fahrenheits(float value) { return static_cast<int>(value * 1.8F) + 32; }

static void check_units_and_convert(DeviceStatus &s) {
  if (!s.tempUnit)
    return;
  s.indoor_temp = celsius_to_fahrenheits(s.indoor_temp);
  s.outdoor_temp = celsius_to_fahrenheits(s.outdoor_temp);
  s.setTemperature = celsius_to_fahrenheits(s.setTemperature + s.setTemperature_dot * 0.5F);
}

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

void DeviceStatus::fromA0Response(DeviceStatus &deviceStatus, const FrameData &data) {
  deviceStatus.powerStatus = data[1] & 1;
  deviceStatus.setTemperature = static_cast<float>(((data[1] >> 1) & 31) + 12);
  deviceStatus.setTemperature_dot = data[1] & 64;
  deviceStatus.errMark = data[1] & 128;
  deviceStatus.mode = data[2] >> 5;
  deviceStatus.fanSpeed = data[3] & 127;
  deviceStatus.timer_on_hour = (data[4] >> 2) & 31;
  deviceStatus.timer_on_min = (data[4] & 3) | (data[6] >> 4);
  deviceStatus.timer_on = data[4] & 128;
  deviceStatus.timer_off_hour = (data[5] >> 2) & 31;
  deviceStatus.timer_off_min = (data[5] & 3) | (data[6] & 15);
  deviceStatus.timer_off = data[5] & 128;
  if (!deviceStatus.timer_off) {
    deviceStatus.timer_off_min = 0;
    deviceStatus.timer_off_hour = 0;
  }
  if (!deviceStatus.timer_on) {
    deviceStatus.timer_on_min = 0;
    deviceStatus.timer_on_hour = 0;
  }
  deviceStatus.updownFan = data[7] & 12;
  deviceStatus.leftRightFan = data[7] & 3;
  deviceStatus.cosySleep = data[8] & 3;
  deviceStatus.save = data[8] & 8;
  deviceStatus.lowFerqFan = data[8] & 16;
  deviceStatus.tubro = data[8] & 32;
  deviceStatus.feelOwn = data[8] & 128;
  deviceStatus.exchangeAir = data[9] & 2;
  deviceStatus.dryClean = data[9] & 4;
  deviceStatus.ptcAssis = data[9] & 8;
  deviceStatus.eco = data[9] & 16;
  deviceStatus.cleanUp = data[9] & 32;
  deviceStatus.tempUnit = data[9] & 128;
  deviceStatus.sleepFunc = data[10] & 1;
  if (!deviceStatus.tubro)
    deviceStatus.tubro = data[10] & 2;
  deviceStatus.catchCold = data[10] & 8;
  deviceStatus.nightLight = data[10] & 16;
  deviceStatus.peakElec = data[10] & 32;
  deviceStatus.naturalFan = data[10] & 64;
  deviceStatus.pwmMode = data[11] & 15;
  deviceStatus.light = (data[11] >> 4) & 7;
  deviceStatus.setExpand_dot = data[12] & 1;
  deviceStatus.setExpand = (data[12] & 31) + 12;
  deviceStatus.double_temp = data[12] & 64;
  deviceStatus.Eight_Hot = data[12] & 128;
  deviceStatus.humidity = data[13] & 127;
  deviceStatus.hasNoWindFeel = data[14] & 8;
  check_units_and_convert(deviceStatus);
}

void DeviceStatus::fromA1Response(DeviceStatus &deviceStatus, const FrameData &data) {
  deviceStatus.indoor_temp = static_cast<float>(data[13] - 50) * 0.5F;
  deviceStatus.outdoor_temp = static_cast<float>(data[14] - 50) * 0.5F;
  deviceStatus.humidity = data[17] & 127;
}

void DeviceStatus::fromC0Response(DeviceStatus &deviceStatus, const FrameData &data) {
  deviceStatus.powerStatus = data[1] & 1;
  deviceStatus.imodeResume = data[1] & 4;
  deviceStatus.timerMode = (data[1] & 16) >> 1;
  deviceStatus.test2 = data[1] & 32;
  deviceStatus.errMark = data[1] & 128;
  deviceStatus.setTemperature = static_cast<float>(data[2] & 15 + 16);
  deviceStatus.setTemperature_dot = data[2] & 16;
  deviceStatus.mode = data[2] >> 5;
  deviceStatus.fanSpeed = data[3] & 127;
  if (deviceStatus.fanSpeed == 101)
    deviceStatus.fanSpeed = 102;
  deviceStatus.timer_on_hour = (data[4] >> 2) & 31;
  deviceStatus.timer_on_min = (data[4] & 3) | (data[6] >> 4);
  deviceStatus.timer_on = data[4] & 128;
  deviceStatus.timer_off_hour = (data[5] & 124) >> 2;
  deviceStatus.timer_off_min = (data[5] & 3) | (data[6] & 15);
  deviceStatus.timer_off = data[5] & 128;
  if (!deviceStatus.timer_off) {
    deviceStatus.timer_off_min = 0;
    deviceStatus.timer_off_hour = 0;
  }
  if (!deviceStatus.timer_on) {
    deviceStatus.timer_on_min = 0;
    deviceStatus.timer_on_hour = 0;
  }
  deviceStatus.updownFan = data[7] & 12;
  deviceStatus.leftRightFan = data[7] & 3;
  deviceStatus.cosySleep = data[8] & 3;
  deviceStatus.save = data[8] & 8;
  deviceStatus.lowFerqFan = data[8] & 16;
  deviceStatus.tubro = data[8] & 32;
  deviceStatus.feelOwn = data[8] & 128;
  deviceStatus.childSleepMode = data[9] & 1;
  deviceStatus.naturalFan = data[9] & 2;
  deviceStatus.dryClean = data[9] & 4;
  deviceStatus.ptcAssis = data[9] & 8;
  deviceStatus.eco = data[9] & 16;
  deviceStatus.cleanUp = data[9] & 32;
  deviceStatus.sleepFunc = data[9] & 64;
  deviceStatus.selfFeelOwn = data[9] & 128;
  deviceStatus.sleepFunc = data[10] & 1;
  if (!deviceStatus.tubro)
    deviceStatus.tubro = data[10] & 2;
  deviceStatus.tempUnit = data[10] & 4;
  deviceStatus.exchangeAir = data[10] & 8;
  deviceStatus.nightLight = data[10] & 16;
  deviceStatus.catchCold = data[10] & 32;
  deviceStatus.peakElec = data[10] & 64;
  deviceStatus.coolFan = data[10] & 128;
  deviceStatus.Eight_Hot = data[12] & 128;
  deviceStatus.dusFull = data[13] & 32;
  deviceStatus.pwmMode = data[14] & 15;
  deviceStatus.light = (data[14] >> 4) & 7;
  deviceStatus.T1_dot = data[15] & 15;
  deviceStatus.T4_dot = data[15] >> 4;
  deviceStatus.indoor_temp = get_temperature(data[11], deviceStatus.T1_dot, deviceStatus.tempUnit);
  deviceStatus.outdoor_temp = get_temperature(data[12], deviceStatus.T4_dot, deviceStatus.tempUnit);
  deviceStatus.errInfo = data[16];
  if (data[13] & 31) {
    deviceStatus.setNewTemperature = static_cast<float>((data[13] & 31) + 12);
    deviceStatus.setTemperature = deviceStatus.setNewTemperature;
  }
  deviceStatus.humidity = data[19] & 127;
  if (data.size() >= 23)
    deviceStatus.hasNoWindFeel = data[22] & 8;
  if (data.size() >= 24) {
    deviceStatus.double_temp = data[21] & 64;
    deviceStatus.Eight_Hot = data[21] & 128;
  } else {
    deviceStatus.double_temp = 0;
    deviceStatus.Eight_Hot = 0;
  }
  check_units_and_convert(deviceStatus);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
