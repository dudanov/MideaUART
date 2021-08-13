#include "Appliance/AirConditioner/StatusData.h"

namespace dudanov {
namespace midea {
namespace ac {

float StatusData::getTargetTemp() const {
  float temp = static_cast<float>(this->m_getValue(2, 0, 0b1111) + 16);
  if (this->m_getValue(2, 0, 0x10))
    temp += 0.5;
  return temp;
}

void StatusData::setTargetTemp(float temp) {
  uint8_t tmp = static_cast<uint8_t>(temp * 16.0) + 4;
  tmp = ((tmp & 8) << 1) | (tmp >> 4);
  this->m_setValue(2, 0, 0x1F, tmp);
}

static float i16tof(int16_t in) { return static_cast<float>(in - 50) * 0.5f; }
float StatusData::getIndoorTemp() const { return i16tof(this->m_getValue(11)); }
float StatusData::getOutdoorTemp() const { return i16tof(this->m_getValue(12)); }
float StatusData::getHumiditySetpoint() const { return static_cast<float>(this->m_getValue(19, 0, 0x7F)); }

Mode StatusData::getMode() const {
  if (!this->m_getPower())
    return Mode::MODE_OFF;
  return static_cast<Mode>(this->m_getValue(2, 5));
}

void StatusData::setMode(Mode mode) {
  if (mode == Mode::MODE_OFF) {
    this->m_setPower(false);
    return;
  }
  this->m_setPower(true);
  this->m_setValue(2, 5, 0b111, mode);
}

Preset StatusData::getPreset() const {
  if (this->m_getEco())
    return Preset::PRESET_ECO;
  if (this->m_getTurbo())
    return Preset::PRESET_TURBO;
  if (this->m_getSleep())
    return Preset::PRESET_SLEEP;
  if (this->m_getFreezeProtection())
    return Preset::PRESET_FREEZE_PROTECTION;
  return Preset::PRESET_NONE;
}

void StatusData::setPreset(Preset preset) {
  this->m_setEco(false);
  this->m_setSleep(false);
  this->m_setTurbo(false);
  this->m_setFreezeProtection(false);
  switch (preset) {
    case Preset::PRESET_NONE:
      break;
    case Preset::PRESET_ECO:
      this->m_setEco(true);
      break;
    case Preset::PRESET_TURBO:
      this->m_setTurbo(true);
      break;
    case Preset::PRESET_SLEEP:
      this->m_setSleep(true);
      break;
    case Preset::PRESET_FREEZE_PROTECTION:
      this->m_setFreezeProtection(true);
      break;
    default:
      break;
  }
}

static uint8_t bcd2u8(uint8_t bcd) { return 10 * (bcd >> 4) + (bcd & 0b1111); }

float StatusData::getPowerUsage() const {
  uint32_t power = 0;
  const uint8_t *ptr = this->m_data.data() + 18;
  for (uint32_t weight = 1;; weight *= 100, --ptr) {
    power += weight * bcd2u8(*ptr);
    if (weight == 10000)
      return static_cast<float>(power) * 0.1f;
  }
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
