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

}  // namespace midea
}  // namespace dudanov
