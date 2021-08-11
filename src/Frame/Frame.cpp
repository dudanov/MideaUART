#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

uint8_t Frame::m_calcCS() const {
  if (this->m_data.size() < 11)
    return 0;
  uint8_t cs = 0;
  for (auto it = this->m_data.begin() + OFFSET_LENGTH; it != this->m_data.end(); ++it)
    cs -= *it;
  return cs;
}

}  // namespace midea
}  // namespace dudanov
