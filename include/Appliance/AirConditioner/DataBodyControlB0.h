#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

typedef unsigned char uint8_t;

class DataBodyControlB0 : public FrameData {
 public:
  DataBodyControlB0(uint8_t funcLo, uint8_t value, uint8_t sound) : DataBodyControlB0(funcLo, value, {}, sound, 0x00) {}

  DataBodyControlB0(uint8_t funcLo, uint8_t value, std::vector<uint8_t> values, uint8_t sound, uint8_t funcHi) {
    if (funcLo == 0x63 || funcLo == 0x64) {
      // Control function: 0x0018: SILKY_COOL (NoWindFeel)
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02, 0x18, 0x00, 0x01, static_cast<uint8_t>(funcLo == 0x63), 0x1A, 0x00, 0x01, sound};
    } else if (funcHi == 0x02 && (funcLo == 0x30 || funcLo == 0x31)) {
      // Control functions: 0x0230, 0x0231
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02, funcLo, 0x02, static_cast<uint8_t>(values.size())};
      this->m_data.insert(this->m_data.end(), values.begin(), values.end());
      this->m_data.insert(this->m_data.end(), {0x1A, 0x00, 0x01, 0x01});
    } else if (funcHi == 0x00 &&
               (funcLo == 0x09 || funcLo == 0x0A || funcLo == 0x30 || funcLo == 0x32 || funcLo == 0x33 ||
                funcLo == 0x39 || funcLo == 0x42 || funcLo == 0x43 || funcLo == 0x67)) {
      // Control functions:
      // 0x0009: V_WIND_DIRECTION, 0x000A: H_WIND_DIRECTION, 0x0030: ECO_EYE, 0x0032: WIND_ON_ME
      // 0x0033: WIND_OFF_ME, 0x0039: ACTIVE_CLEAN, 0x0042: BREEZE_AWAY
      // 0x0043: BREEZE, 0x0067: JET_COOL
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02, funcLo, 0x00, 0x01, value, 0x1A, 0x00, 0x01, 0x01};
    } else if (funcHi == 0x02 && funcLo == 0x2C) {
      // Control: Buzzer
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x01, 0x2C, 0x02, 0x01, value, sound};
    } else if (funcHi == 0x00 && funcLo == 0x5B) {
      // Device: 0xA1, FrameType: 0x02
      this->m_data = {0xB0, 0x01, 0x5B, 0x00, 0x01, value, sound};
    } else if (funcHi == 0x00 && funcLo == 0x15) {
      // Query: Indoor humidity
      // Device: 0xAC, FrameType: 0x03
      this->m_data = {0xB1, 0x01, 0x15, 0x00};
    } else if (funcHi == 17 && funcLo == 0x5B) {
      //
      // Device: 0xA1, FrameType: 0x03
      this->m_data = {0xB1, 0x01, 0x5B, 0x00};
    } else if (funcHi == 18 && funcLo == 50) {
      // Query: "Wind ON me", "Wind OFF me"
      // Device: 0xAC, FrameType: 0x03
      this->m_data = {0xB1, 0x02, 0x32, 0x00, 0x33, 0x00};
    } else if (funcLo == 101) {
      // Control function:
      // 0x004B: FRESH_AIR
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x01, 0x4B, 0x00, 0x04, values[0], values[1], 0xFF, 0xFF};
    } else if (funcLo == 102 || funcLo == 103) {
      // Control functions 102, 103:
      // 0x004B: FRESH_AIR [0x01, 0x03, 0xFF, 0xFF], 0x0043: BREEZE [0x04]
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02, 0x4B, 0x00, 0x04, 0x01, 0x03, 0xFF, 0xFF, 0x43, 0x00, 0x01, 0x04};
    } else if (funcLo == 105) {
      // Control function 105:
      // 0x0043: BREEZE [0x04]
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02 /* or 0x01 ? */, 0x43, 0x00, 0x01, 0x04};
    } else if (funcLo == 106) {
      // Device: 0xAC, FrameType: 0x02
      this->m_data = {0xB0, 0x02, 0x4B, 0x00, 0x04, 0x01, 0x03, 0xFF, 0xFF, 0x43, 0x00, 0x01, 0x03};
    }
    this->appendCRC();
  }
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
