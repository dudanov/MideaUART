#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

void Frame::setData(const FrameData &data) {
  this->m_trimData();
  this->m_appendData(data);
  this->m_data[OFFSET_LENGTH] = this->m_data.size();
  this->m_data[OFFSET_SYNC] = this->m_data[OFFSET_LENGTH] ^ this->m_data[OFFSET_APPTYPE];
  this->m_appendCS();
}

uint8_t Frame::m_calcCS() const {
  if (this->m_data.size() <= OFFSET_DATA)
    return 0;
  uint8_t cs = 0;
  for (auto it = this->m_data.begin() + OFFSET_LENGTH; it != this->m_data.end(); ++it)
    cs -= *it;
  return cs;
}

static char u4hex(uint8_t num) { return num + ((num < 10) ? '0' : ('A' - 10)); }

String Frame::toString() const {
  String ret;
  char buf[4];
  buf[2] = ' ';
  buf[3] = '\0';
  ret.reserve(3 * this->size());
  for (const uint8_t data : this->m_data) {
    buf[0] = u4hex(data / 16);
    buf[1] = u4hex(data % 16);
    ret += buf;
  }
  return ret;
}

}  // namespace midea
}  // namespace dudanov
