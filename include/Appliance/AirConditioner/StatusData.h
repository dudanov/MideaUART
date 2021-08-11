#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"
#include "Appliance/ApplianceBase.h"

namespace dudanov {
namespace midea {
namespace ac {

class StatusData : public FrameData {
 public:
  StatusData() : FrameData({0x40, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00}) {}
  StatusData(const FrameData &data) : FrameData(data) {}

  /// Copy status from another StatusData
  void copyStatus(const StatusData &p) { memcpy(this->m_data.data() + 1, p.data() + 1, 10); }

  bool hasStatus() const { return this->hasID(0xC0); }
  bool hasPowerInfo() const { return this->hasID(0xC1); }

  /* TARGET TEMPERATURE */
  float getTargetTemp() const;
  void setTargetTemp(float temp);

  /* MODE */
  Mode getMode() const;
  void setMode(Mode mode);

  /* FAN SPEED */
  FanMode getFanMode() const { return static_cast<FanMode>(this->m_data[3]); }
  void setFanMode(FanMode mode) { this->m_data[3] = mode; };

  /* SWING MODE */
  SwingMode getSwingMode() const { return static_cast<SwingMode>(this->m_getValue(7, 0, 0b1111)); }
  void setSwingMode(SwingMode mode) { this->m_data[7] = 0x30 | mode; }

  /* INDOOR TEMPERATURE */
  float getIndoorTemp() const;

  /* OUTDOOR TEMPERATURE */
  float getOutdoorTemp() const;

  /* HUMIDITY SETPOINT */
  float getHumiditySetpoint() const;

  /* PRESET */
  Preset getPreset() const;
  void setPreset(Preset preset);

  /* POWER USAGE */
  float getPowerUsage() const;

  void setBeeper(bool state) { this->m_setByteMask(1, 0x40, state); }

 protected:
  /* POWER */
  bool m_getPower() const { return this->m_data[1] & 0x01; }
  void m_setPower(bool state) { this->m_setByteMask(1, 0x01, state); }
  /* ECO MODE */
  bool m_getEco() const { return this->m_data[9] & 0x10; }
  void m_setEco(bool state) { this->m_setByteMask(9, 0x80, state); }
  /* TURBO MODE */
  bool m_getTurbo() const { return this->m_data[8] & 0x20 || this->m_data[10] & 0x02; }
  void m_setTurbo(bool state) {
    this->m_setByteMask(8, 0x20, state);
    this->m_setByteMask(10, 0x02, state);
  }
  /* FREEZE PROTECTION */
  bool m_getFreezeProtection() const { return this->m_data[21] & 0x80; }
  void m_setFreezeProtection(bool state) { this->m_setByteMask(21, 0x80, state); }
  /* SLEEP MODE */
  bool m_getSleep() const { return this->m_data[10] & 0x01; }
  void m_setSleep(bool state) { this->m_setByteMask(10, 0x01, state); }
};


class QueryStateData : public FrameData {
 public:
  QueryStateData() : FrameData({0x41, 0x81, 0x00, 0xFF, 0x03, 0xFF, 0x00, 0x02, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x03, FrameData::m_getID()}) { this->appendCRC(); }
};

class QueryPowerData : public FrameData {
 public:
  QueryPowerData() : FrameData({0x41, 0x21, 0x01, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x04, FrameData::m_getID()}) { this->appendCRC(); }
};

class DisplayToggleData : public FrameData {
 public:
  DisplayToggleData() : FrameData({0x41, 0x61, 0x00, 0xFF, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, FrameData::m_getRandom()}) { this->appendCRC(); }
};

class GetCapabilitiesData : public FrameData {
 public:
  GetCapabilitiesData() : FrameData({0xB5, 0x01, 0x11}) { this->appendCRC(); }
};

class GetCapabilitiesSecondData : public FrameData {
 public:
  GetCapabilitiesSecondData() : FrameData({0xB5, 0x01, 0x01, 0x00}) { this->appendCRC(); }
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
