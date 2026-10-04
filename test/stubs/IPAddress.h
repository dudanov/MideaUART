#pragma once
#include <cstdint>

class IPAddress {
 public:
  uint8_t operator[](int) const { return 0; }
};
