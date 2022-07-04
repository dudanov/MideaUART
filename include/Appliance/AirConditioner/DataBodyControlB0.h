#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"

namespace dudanov {
namespace midea {
namespace ac {

typedef unsigned char uint8_t;

class DataBodyControlB0 : public FrameData {
 public:
  DataBodyControlB0(uint8_t cmd, uint8_t value, uint8_t sound) : DataBodyControlB0(cmd, value, {}, sound, 0) {}
  DataBodyControlB0(uint8_t cmd, uint8_t value, std::vector<uint8_t> values, uint8_t sound, uint8_t cmdType) {
    if (cmd == 99 || cmd == 100) {
      this->m_data = {-80, 2, 24, 0, 1, cmd == 99, 26, 0, 1, sound};
    } else if (cmdType == 2 && (cmd == 48 || cmd == 49)) {
      this->m_data = {-80, 2, cmd, 2, values.size()};
      this->m_data.insert(this->m_data.end(), values.begin(), values.end());
      this->m_data.insert(this->m_data.end(), {26, 0, 1, 1});
    } else if (cmdType == 0 && (cmd == 50 || cmd == 51 || cmd == 57 || cmd == 66 || cmd == 67 || cmd == 48 ||
                                cmd == 9 || cmd == 10 || cmd == 103)) {
      this->m_data = {-80, 2, cmd, 0, 1, value, 26, 0, 1, 1};
    } else if (cmdType == 2 && cmd == 44) {
      this->m_data = {-80, 1, 44, 2, 1, value, sound};
    } else if (cmdType == 0 && cmd == 91) {
      this->m_data = {-80, 1, 91, 0, 1, value, sound};
    } else if (cmdType == 0 && cmd == 21) {
      this->m_data = {-79, 1, 21, 0};
    } else if (cmdType == 17 && cmd == 91) {
      this->m_data = {-79, 1, 91, 0};
    } else if (cmdType == 18 && cmd == 50) {
      this->m_data = {-79, 2, 50, 0, 51, 0};
    } else if (cmd == 101) {
      this->m_data = {-80, 1, 75, 0, 4, values[0], values[1], -1, -1};
    } else if (cmd == 102) {
      this->m_data = {-80, 2, 75, 0, 4, 1, 3, -1, -1, 67, 0, 1, 4};
    } else if (cmd == 103) {
      this->m_data = {-80, 2, 75, 0, 4, 1, 3, -1, -1, 67, 0, 1, 4};
    } else if (cmd == 105) {
      this->m_data = {-80, 2, 67, 0, 1, 4};
    } else if (cmd == 106) {
      this->m_data = {-80, 2, 75, 0, 4, 1, 3, -1, -1, 67, 0, 1, 3};
    }
    this->appendCRC();
  }
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
