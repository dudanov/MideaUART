#include "IRProtocol.h"

namespace dudanov {
namespace midea {

static const char *const TAG = "remote.midea";

// Reverse bits in byte
static uint8_t s_reverse(uint8_t data) {
  static const uint8_t PROGMEM table[] = {
      0b0000, 0b1000, 0b0100, 0b1100, 0b0010, 0b1010, 0b0110, 0b1110,
      0b0001, 0b1001, 0b0101, 0b1101, 0b0011, 0b1011, 0b0111, 0b1111,
  };
  return pgm_read_byte(table + data % 16) * 16 + pgm_read_byte(table + data / 16);
}

uint8_t IRData::m_calcCS() const {
  uint8_t cs = 0;
  for (const uint8_t *it = this->data(); it != this->data() + OFFSET_CS; ++it)
    cs -= s_reverse(*it);
  return s_reverse(cs);
}

}  // namespace midea
}  // namespace dudanov
