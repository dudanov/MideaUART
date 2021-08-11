#pragma once
#include <Arduino.h>

namespace dudanov {
namespace midea {

class IRData {
 public:
  IRData(std::initializer_list<uint8_t> data) { std::copy(data.begin(), data.end(), this->data()); }
  IRData(const IRData &) = default;

  uint8_t *data() { return this->m_data; }
  const uint8_t *data() const { return this->m_data; }
  uint8_t size() const { return sizeof(this->m_data); }
  void finalize() { this->m_data[OFFSET_CS] = this->m_calcCS(); }
  enum IRDataType : uint8_t {
    MIDEA_TYPE_COMMAND = 0xA1,
    MIDEA_TYPE_SPECIAL = 0xA2,
    MIDEA_TYPE_FOLLOW_ME = 0xA4,
  };
  IRDataType type() const { return static_cast<IRDataType>(this->m_data[0]); }
  template<typename T> T to() const { return T(*this); }

 protected:
  void m_setValue(uint8_t offset, uint8_t val_mask, uint8_t shift, uint8_t val) {
    m_data[offset] &= ~(val_mask << shift);
    m_data[offset] |= (val << shift);
  }
  static const uint8_t OFFSET_CS = 5;
  // 48-bits data
  uint8_t m_data[6];
  // Calculate checksum
  uint8_t m_calcCS() const;
};


class IrFollowMeData : public IRData {
 public:
  // Default constructor (temp: 30C, beeper: off)
  IrFollowMeData() : IRData({MIDEA_TYPE_FOLLOW_ME, 0x82, 0x48, 0x7F, 0x1F}) {}
  // Copy from Base
  IrFollowMeData(const IRData &data) : IRData(data) {}
  // Direct from temperature and beeper values
  IrFollowMeData(uint8_t temp, bool beeper = false) : IrFollowMeData() {
    this->setTemp(temp);
    this->setBeeper(beeper);
  }

  /* TEMPERATURE */
  uint8_t temp() const { return this->m_data[4] - 1; }
  void setTemp(uint8_t val) { this->m_data[4] = std::min(MAX_TEMP, val) + 1; }

  /* BEEPER */
  bool beeper() const { return this->m_data[3] & 128; }
  void setBeeper(bool val) { this->m_setValue(3, 1, 7, val); }

 protected:
  static const uint8_t MAX_TEMP = 37;
};

class IrSpecialData : public IRData {
 public:
  IrSpecialData(uint8_t code) : IRData({MIDEA_TYPE_SPECIAL, code, 0xFF, 0xFF, 0xFF}) {}
};

#if 0
class IrCommandData : public IRData {
 public:
  // Default constructor: power: on, mode: auto, fan: auto, temp: 25C, all timers: off
  IrCommandData() : IRData({MIDEA_TYPE_COMMAND, 0x82, 0x48, 0xFF, 0xFF}) {}
  // Copy from Base
  IrCommandData(const IRData &data) : IRData(data) {}
  // Midea modes enum
  enum MideaMode : uint8_t {
    MIDEA_MODE_COOL,
    MIDEA_MODE_DRY,
    MIDEA_MODE_AUTO,
    MIDEA_MODE_HEAT,
    MIDEA_MODE_FAN,
  };
  // Midea fan enum
  enum MideaFan : uint8_t {
    MIDEA_FAN_AUTO,
    MIDEA_FAN_LOW,
    MIDEA_FAN_MEDIUM,
    MIDEA_FAN_HIGH,
  };
  // Set temperature setpoint
  void set_temp(uint8_t val) { this->data_[2] = std::min(MAX_TEMP, std::max(MIN_TEMP, val)) - MIN_TEMP; }
  // Set mode
  void set_mode(MideaMode mode) { m_setValue(1, 0b111, 0, mode); }
  // Set fan speed
  void set_fan_speed(MideaFan fan) { m_setValue(1, 0b11, 3, fan); }
  // Set sleep
  void set_sleep(bool val) { m_setValue(1, 0b1, 6, val); }
  // Set power
  void set_power(bool val) { m_setValue(1, 0b1, 7, val); }
  // Set ON timer (0: disable)
  void set_on_timer(uint16_t minutes) {
    uint8_t halfhours = std::min<uint16_t>(24 * 60, minutes) / 30;
    this->data_[4] = halfhours ? ((halfhours - 1) * 2 + 1) : 0xFF;
  }
  // Set OFF timer (0: disable)
  void set_off_timer(uint16_t minutes) {
    uint8_t halfhours = std::min<uint16_t>(24 * 60, minutes) / 30;
    this->m_setValue(3, 0b111111, 1, halfhours ? (halfhours - 1) : 0b111111);
  }

 protected:
  static const uint8_t MIN_TEMP = 17;
  static const uint8_t MAX_TEMP = 30;
};
#endif

}  // namespace midea_ac
}  // namespace esphome
