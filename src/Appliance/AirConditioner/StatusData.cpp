#include "Appliance/AirConditioner/StatusData.h"
#include "Appliance/AirConditioner/Capabilities.h"

namespace dudanov {
namespace midea {
namespace ac {

bool inRange(float d) { return d > 17 && d < 30; }

bool isSpecialValue(float d) {
  if (d == 2.5f || d == 7.5f || d == 12.5f)
    return true;

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
  float temp = static_cast<float>(m_getValue(2, 15) + 16);

  if (m_getValue(2, 16))
    temp += 0.5f;

  return temp;
}

void StatusData::setTargetTemp(float temp) {
  uint8_t tmp = static_cast<uint8_t>(temp * 16.0f) + 4;
  m_setValue(2, ((tmp & 8) << 1) | (tmp >> 4), 31);
}

static float i16tof(int16_t in) { return static_cast<float>(in - 50) * 0.5f; }
float StatusData::getIndoorTemp() const { return i16tof(m_getValue(11)); }
float StatusData::getOutdoorTemp() const { return i16tof(m_getValue(12)); }
float StatusData::getHumiditySetpoint() const { return static_cast<float>(m_getValue(19, 127)); }

Mode StatusData::getMode() const { return m_getPower() ? this->getRawMode() : Mode::MODE_OFF; }

void StatusData::setMode(Mode mode) {
  if (mode == Mode::MODE_OFF) {
    m_setPower(false);
    return;
  }

  m_setPower(true);
  m_setValue(2, mode, 7, 5);
}

FanSpeed StatusData::getFanMode() const {
  const auto speed = static_cast<FanSpeed>(m_getValue(3));

  if (speed <= FanSpeed::FAN_LOW)
    return FanSpeed::FAN_LOW;

  if (speed <= FanSpeed::FAN_MEDIUM)
    return FanSpeed::FAN_MEDIUM;

  return speed;
}

Preset StatusData::getPreset() const {
  if (m_getEco())
    return Preset::PRESET_ECO;

  if (m_getTurbo())
    return Preset::PRESET_TURBO;

  if (m_getSleep())
    return Preset::PRESET_SLEEP;

  if (m_getFreezeProtection())
    return Preset::PRESET_FREEZE_PROTECTION;

  return Preset::PRESET_NONE;
}

void StatusData::setPreset(Preset preset) {
  m_setEco(false);
  m_setSleep(false);
  m_setTurbo(false);
  m_setFreezeProtection(false);

  switch (preset) {
    case Preset::PRESET_NONE:
      break;

    case Preset::PRESET_ECO:
      m_setEco(true);
      break;

    case Preset::PRESET_TURBO:
      m_setTurbo(true);
      break;

    case Preset::PRESET_SLEEP:
      m_setSleep(true);
      break;

    case Preset::PRESET_FREEZE_PROTECTION:
      m_setFreezeProtection(true);
      break;

    default:
      break;
  }
}

static uint8_t bcd2u8(uint8_t bcd) { return bcd / 16 * 10 + bcd % 16; }

float StatusData::getPowerUsage() const {
  uint32_t result = 0;
  const uint8_t *it = &m_data[18];

  for (uint32_t weight = 1;; weight *= 100, --it) {
    result += weight * bcd2u8(*it);

    if (weight == 10000)
      return static_cast<float>(result) * 0.1f;
  }
}

/* GetFeatureStateQuery */

GetFeatureStateQuery::GetFeatureStateQuery() : PropertiesData(0xB1) {
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

GetFeatureStateQuery::GetFeatureStateQuery(const CmdB5 &b5) : PropertiesData(0xB1) {
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

  this->appendCRC();
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
