#include "Appliance/AirConditioner/DeviceStatus.h"
#include "Appliance/AirConditioner/StatusData.h"

namespace dudanov {
namespace midea {
namespace ac {

static int celsius_to_fahrenheits(float value) { return static_cast<int>(value * 1.8f) + 32; }
static float fahrenheits_to_celsius(int value) { return static_cast<float>(value - 32) / 1.8f; }

static float s_get_temperature(int integer, int decimal, bool fahrenheits) {
  integer -= 50;

  if (!fahrenheits && decimal > 0)
    return static_cast<float>(integer / 2) + decimal * ((integer >= 0) ? 0.1f : -0.1f);

  if (decimal >= 5)
    return static_cast<float>(integer / 2) + ((integer >= 0) ? 0.5f : -0.5f);

  return static_cast<float>(integer) * 0.5f;
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
  this->turbo = deviceStatus.turbo;
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

DeviceStatus FrameStatusData::updateFromA0() {
  DeviceStatus status{};

  // Byte #1
  status.powerStatus = this->m_getBit(1, 0);
  status.setTemperature = this->m_getValue(1, 31, 1) + 12;
  status.setTemperature_dot = this->m_getBit(1, 6);
  status.errMark = this->m_getBit(1, 7);

  // Byte #2
  status.mode = this->m_getValue(2, 7, 5);

  // Byte #3
  status.fanSpeed = this->m_getValue(3, 127);

  // Byte #4,6
  status.timer_on_hour = this->m_getValue(4, 31, 2);
  status.timer_on_min = this->m_getValue(4, 3) | this->m_getValue(6, 15, 4);
  status.timer_on = this->m_getBit(4, 7);

  // Byte #5,6
  status.timer_off_hour = this->m_getValue(5, 31, 2);
  status.timer_off_min = this->m_getValue(5, 3) | this->m_getValue(6, 15);
  status.timer_off = this->m_getBit(5, 7);

  if (!status.timer_off) {
    status.timer_off_min = 0;
    status.timer_off_hour = 0;
  }

  if (!status.timer_on) {
    status.timer_on_min = 0;
    status.timer_on_hour = 0;
  }

  // Byte #7
  status.updownFan = this->m_getValue(7, 3, 2);
  status.leftRightFan = this->m_getValue(7, 3);

  // Byte #8
  status.cosySleep = this->m_getValue(8, 3);
  status.save = this->m_getBit(8, 3);
  status.lowFerqFan = this->m_getBit(8, 4);
  status.turbo = this->m_getBit(8, 5);
  status.feelOwn = this->m_getBit(8, 7);

  // Byte #9
  status.exchangeAir = this->m_getBit(9, 1);
  status.dryClean = this->m_getBit(9, 2);
  status.ptcAssis = this->m_getBit(9, 3);
  status.eco = this->m_getBit(9, 4);
  status.cleanUp = this->m_getBit(9, 5);
  status.tempUnit = this->m_getBit(9, 7);

  // Byte #10
  status.sleepFunc = this->m_getBit(10, 0);

  if (!status.turbo)
    status.turbo = this->m_getBit(10, 1);

  status.catchCold = this->m_getBit(10, 3);
  status.nightLight = this->m_getBit(10, 4);
  status.peakElec = this->m_getBit(10, 5);
  status.naturalFan = this->m_getBit(10, 6);

  // Byte #11
  status.pwmMode = this->m_getValue(11, 15);
  status.light = this->m_getValue(11, 7, 4);

  // Byte #12
  status.setExpand_dot = this->m_getBit(12, 0);
  status.setExpand = this->m_getValue(12, 31) + 12;
  status.double_temp = this->m_getBit(12, 6);
  status.Eight_Hot = this->m_getBit(12, 7);

  // Byte #13
  status.humidity = this->m_getValue(13, 127);

  // Byte #14
  status.hasNoWindFeel = this->m_getBit(14, 3);

  if (status.tempUnit)
    status.convertUnits();
}

DeviceStatus FrameStatusData::updateFromA1() {
  DeviceStatus status{};
  status.indoor_temp = static_cast<float>(this->m_getValue(13) - 50) * 0.5f;
  status.outdoor_temp = static_cast<float>(static_cast<int8_t>(this->m_getValue(14)) - 50) * 0.5f;
  status.humidity = this->m_getValue(17, 127);
}

DeviceStatus FrameStatusData::updateFromC0() {
  DeviceStatus status{};

  // Byte #1
  status.powerStatus = this->m_getBit(1, 0);
  status.imodeResume = this->m_getBit(1, 2);
  status.timerMode = this->m_getBit(1, 4);
  status.test2 = this->m_getBit(1, 5);
  status.errMark = this->m_getBit(1, 7);

  // Byte #2
  status.setTemperature = this->m_getValue(2, 15) + 16;
  status.setTemperature_dot = this->m_getBit(2, 4);
  status.mode = this->m_getValue(2, 7, 5);

  // Byte #3
  status.fanSpeed = this->m_getValue(3, 127);

  if (status.fanSpeed == FAN_FIXED)
    status.fanSpeed = FAN_AUTO;

  // Bytes #4,6
  status.timer_on_hour = this->m_getValue(4, 31, 2);
  status.timer_on_min = this->m_getValue(4, 3) | this->m_getValue(6, 15, 4);
  status.timer_on = this->m_getBit(4, 7);

  // Bytes #5,6
  status.timer_off_hour = this->m_getValue(5, 31, 2);
  status.timer_off_min = this->m_getValue(5, 3) | this->m_getValue(6, 15);
  status.timer_off = this->m_getBit(5, 7);

  if (!status.timer_off) {
    status.timer_off_min = 0;
    status.timer_off_hour = 0;
  }

  if (!status.timer_on) {
    status.timer_on_min = 0;
    status.timer_on_hour = 0;
  }

  // Byte #7
  status.updownFan = this->m_getValue(7, 3, 2);
  status.leftRightFan = this->m_getValue(7, 3);

  // Byte #8
  status.cosySleep = this->m_getValue(8, 3);
  status.save = this->m_getBit(8, 3);
  status.lowFerqFan = this->m_getBit(8, 4);
  status.turbo = this->m_getBit(8, 5);
  status.feelOwn = this->m_getBit(8, 7);

  // Byte #9
  status.childSleepMode = this->m_getBit(9, 0);
  status.naturalFan = this->m_getBit(9, 1);
  status.dryClean = this->m_getBit(9, 2);
  status.ptcAssis = this->m_getBit(9, 3);
  status.eco = this->m_getBit(9, 4);
  status.cleanUp = this->m_getBit(9, 5);
  status.sleepFunc = this->m_getBit(9, 6);
  status.selfFeelOwn = this->m_getBit(9, 7);

  // Byte #10
  status.sleepFunc = this->m_getBit(10, 0);

  if (!status.turbo)
    status.turbo = this->m_getBit(10, 1);

  status.tempUnit = this->m_getBit(10, 2);
  status.exchangeAir = this->m_getBit(10, 3);
  status.nightLight = this->m_getBit(10, 4);
  status.catchCold = this->m_getBit(10, 5);
  status.peakElec = this->m_getBit(10, 6);
  status.coolFan = this->m_getBit(10, 7);

  // Byte #11
  auto indoor_temp = this->m_getValue(11);

  // Byte #12
  auto outdoor_temp = this->m_getValue(12, 127);
  status.Eight_Hot = this->m_getBit(12, 7);

  // Byte #13
  auto new_temp = this->m_getValue(13, 31);
  status.dusFull = this->m_getBit(13, 5);

  if (new_temp) {
    status.setNewTemperature = new_temp + 12;
    status.setTemperature = status.setNewTemperature;
  }

  // Byte #14
  status.pwmMode = this->m_getValue(14, 15);
  status.light = this->m_getValue(14, 7, 4);

  // Byte #15
  status.T1_dot = this->m_getValue(15, 15);
  status.T4_dot = this->m_getValue(15, 15, 4);

  status.indoor_temp = s_get_temperature(indoor_temp, status.T1_dot, status.tempUnit);
  status.outdoor_temp = s_get_temperature(outdoor_temp, status.T4_dot, status.tempUnit);

  // Byte #16
  status.errInfo = this->m_getValue(16);

  // Byte #19
  status.humidity = this->m_getValue(19, 127);

  // Byte #21
  if (this->size() >= 24) {
    status.double_temp = this->m_getBit(21, 6);
    status.Eight_Hot = this->m_getBit(21, 7);

  } else {
    status.double_temp = 0;
    status.Eight_Hot = 0;
  }

  // Byte #22
  if (this->size() >= 23)
    status.hasNoWindFeel = this->m_getBit(22, 3);

  if (status.tempUnit)
    status.convertUnits();

  return status;
}

void FrameStatusData::to40Command(const DeviceStatus &s) {
  this->m_data = {0x40, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  auto fan_speed = s.fanSpeed;
  auto turbo = s.turbo;
  auto eco = s.eco;

  if (!s.powerStatus || s.mode == MODE_FAN_ONLY) {
    eco = false;
    turbo = false;
  }

  if (s.mode != MODE_DRY && fan_speed == FAN_FIXED)
    fan_speed = FAN_AUTO;

  this->m_data[1] =
      0x42 + s.test2 * 0x20 + s.timerMode * 0x10 + s.childSleepMode * 0x08 + s.imodeResume * 0x04 + s.powerStatus;

  auto set_temp = s.setTemperature;
  bool set_temp_dot = s.setTemperature_dot;

  if (set_temp >= 50) {
    auto tmp = static_cast<uint8_t>(fahrenheits_to_celsius(set_temp) * 2.0f + 0.5f);

    set_temp = tmp / 2;
    set_temp_dot = tmp % 2;
  }

  auto set_temp_new = (set_temp - 12) % 32;

  set_temp -= 16;

  if (set_temp < 1 || set_temp > 14)
    set_temp = 1;

  this->m_data[2] = s.mode * 0x20 + set_temp_dot * 0x10 + set_temp;
  this->m_data[3] = s.timerEffe * 0x80 + fan_speed % 128;

  // Setting timers. Initialized off. Therefore, we process only if enabled.

  if (s.timer_off) {
    this->m_data[5] = 0x80 + s.timer_off_hour * 0x04 + s.timer_off_min / 15;
    this->m_data[6] = s.timer_off_min % 15;
  }

  if (s.timer_on) {
    this->m_data[4] = 0x80 + s.timer_on_hour * 0x04 + s.timer_on_min / 15;
    this->m_data[6] |= s.timer_on_min % 15 * 0x10;
  }

  this->m_data[7] = 0x30 + s.updownFan * 0x0C + s.leftRightFan * 0x03;
  this->m_data[8] = s.feelOwn * 0x80 + s.powerSaver * 0x40 + turbo * 0x20 + s.lowFerqFan * 0x10 + s.save * 0x08 +
                    s.alarmSleep * 0x04 + s.cosySleep % 4;
  this->m_data[9] = eco * 0x80 + s.changeCosySleep * 0x40 + s.cleanUp * 0x20 + s.ptcButton * 0x10 + s.ptcAssis * 0x08 +
                    s.dryClean * 0x04 + s.exchangeAir * 0x02 + s.wiseEye;
  this->m_data[10] = s.cleanFanTime * 0x80 + s.dusFull * 0x40 + s.peakElec * 0x20 + s.nightLight * 0x10 +
                     s.catchCold * 0x08 + s.tempUnit * 0x04 + turbo * 0x02 + s.sleepFunc;
  this->m_data[15] = s.naturalFan * 0x40;
  this->m_data[18] = set_temp_new;
  this->m_data[19] = s.humidity;
  this->m_data[21] = s.Eight_Hot * 0x80 + s.double_temp * 0x40 + s.setExpand % 32 * 0x02 + s.setExpand_dot;
  this->m_data[23] = this->m_getID();

  this->appendCRC();
}

void DeviceStatus::convertUnits() {
  this->indoor_temp = celsius_to_fahrenheits(this->indoor_temp);
  this->outdoor_temp = celsius_to_fahrenheits(this->outdoor_temp);
  this->setTemperature = celsius_to_fahrenheits(this->setTemperature + this->setTemperature_dot * 0.5f);
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
