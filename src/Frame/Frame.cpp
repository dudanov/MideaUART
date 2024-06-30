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
  uint8_t cs = START_BYTE;  // start byte not included in checksum

  for (uint8_t data : this->m_data)
    cs -= data;

  return cs;
}

static char u4hex(uint8_t num) { return num + ((num < 10) ? '0' : ('A' - 10)); }

std::string Frame::toString() const {
  std::string ret;

  if (this->m_data.empty())
    return ret;

  ret.reserve(3 * this->m_data.size());

  for (auto it = this->m_data.begin();;) {
    ret.push_back(u4hex(*it / 16));
    ret.push_back(u4hex(*it % 16));

    if (++it == this->m_data.end())
      return ret;

    ret.push_back(' ');
  }
}

}  // namespace midea
}  // namespace dudanov
