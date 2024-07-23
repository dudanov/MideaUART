#include "Appliance/AirConditioner/StatusData.h"
#include "Appliance/AirConditioner/Capabilities.h"

namespace dudanov {
namespace midea {
namespace ac {

bool inRange(float d) { return d > 17 && d < 30; }

bool isSpecialValue(float d) {
  if (d == 2.5f || d == 7.5f || d == 12.5f) {
    return true;
  }
  return (d >= 15.5f && d <= 17.5f) || d == 32.5f || d == 37.5f || d == 42.5f || d == 47.5f;
}

uint8_t get_louver_position(uint8_t position) {
  if (position == 0)
    return 1;

  if (position >= 4)
    return 100;

  return position * 25;
}

float StatusData::getTargetTemp() const {
  float temp = static_cast<float>(this->m_getValue(2, 15) + 16);
  if (this->m_getValue(2, 16))
    temp += 0.5f;
  return temp;
}

void StatusData::setTargetTemp(float temp) {
  uint8_t tmp = static_cast<uint8_t>(temp * 16.0f) + 4;
  this->m_setValue(2, ((tmp & 8) << 1) | (tmp >> 4), 31);
}

static float i16tof(int16_t in) { return static_cast<float>(in - 50) * 0.5f; }
float StatusData::getIndoorTemp() const { return i16tof(this->m_getValue(11)); }
float StatusData::getOutdoorTemp() const { return i16tof(this->m_getValue(12)); }
float StatusData::getHumiditySetpoint() const { return static_cast<float>(this->m_getValue(19, 127)); }

Mode StatusData::getMode() const { return this->m_getPower() ? this->getRawMode() : Mode::MODE_OFF; }

void StatusData::setMode(Mode mode) {
  if (mode != Mode::MODE_OFF) {
    this->m_setPower(true);
    this->m_setValue(2, mode, 7, 5);
  } else {
    this->m_setPower(false);
  }
}

FanSpeed StatusData::getFanMode() const {
  const auto speed = static_cast<FanSpeed>(this->m_getValue(3));

  if (speed <= FanSpeed::FAN_LOW)
    return FanSpeed::FAN_LOW;

  if (speed <= FanSpeed::FAN_MEDIUM)
    return FanSpeed::FAN_MEDIUM;

  return speed;
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

static uint8_t bcd2u8(uint8_t bcd) { return 10 * (bcd >> 4) + (bcd & 15); }

float StatusData::getPowerUsage() const {
  uint32_t power = 0;
  const uint8_t *ptr = this->m_data.data() + 18;
  for (uint32_t weight = 1;; weight *= 100, --ptr) {
    power += weight * bcd2u8(*ptr);
    if (weight == 10000)
      return static_cast<float>(power) * 0.1f;
  }
}

/* GetFeatureStateQuery */

GetFeatureStateQuery::GetFeatureStateQuery() : PropertyQuery(0xB1) {
  this->appendUUID(INDOOR_HUMIDITY);
  this->appendUUID(SILKY_COOL);
  this->appendUUID(0x1A);
  this->appendUUID(ECO_EYE);
  this->appendUUID(ACTIVE_CLEAN);
  this->appendUUID(BREEZE_AWAY);
  this->appendUUID(BREEZE);
  this->appendUUID(BUZZER);
  this->appendCRC();
}

GetFeatureStateQuery::GetFeatureStateQuery(const CmdB5 &b5) : PropertyQuery(0xB1) {
  if (b5.hasBlowingPeople)
    this->appendUUID(Feature::WIND_ON_ME);

  if (b5.hasAvoidPeople)
    this->appendUUID(Feature::WIND_OFF_ME);

  if (b5.hasSelfClean)
    this->appendUUID(Feature::ACTIVE_CLEAN);

  if (b5.hasOneKeyNoWindOnMe)
    this->appendUUID(Feature::BREEZE_AWAY);

  if (b5.hasBreeze)
    this->appendUUID(Feature::BREEZE);

  if (b5.hasSmartEye)
    this->appendUUID(Feature::ECO_EYE);

  if (b5.hasBuzzer)
    this->appendUUID(Feature::BUZZER);

  if (b5.hasAutoClearHumidity || b5.hasHandClearHumidity)
    this->appendUUID(Feature::INDOOR_HUMIDITY);

  if (b5.hasVerticalWind)
    this->appendUUID(Feature::V_WIND_DIRECTION);

  if (b5.hasHorizontalWind)
    this->appendUUID(Feature::H_WIND_DIRECTION);

  if (b5.isTwins)
    this->appendUUID(2 * 256 + 49);

  if (b5.isFourDirection)
    this->appendUUID(2 * 256 + 48);

  if (b5.hasJetCool)
    this->appendUUID(Feature::JET_COOL);

  if (b5.hasFreshAir)
    this->appendUUID(Feature::FRESH_AIR);

  this->appendCRC();
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
