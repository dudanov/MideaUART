#pragma once
#include "FrameData.h"

class IPAddress;

namespace dudanov {
namespace midea {

class NetworkNotifyData : public FrameData {
 public:
  NetworkNotifyData()
      : FrameData({0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

  void setConnected(bool state) { m_setMask(8, !state, 1); }

  void setSignalStrength(uint8_t value) { m_setValue(2, value); }

  void setIP(const IPAddress &ip);
};

}  // namespace midea
}  // namespace dudanov
