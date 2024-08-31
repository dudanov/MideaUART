#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

void Frame::setData(const FrameData &data) {
  m_trimData();
  m_appendData(data);
  m_data[OFFSET_LENGTH] = m_data.size();
  m_data[OFFSET_SYNC] = m_data[OFFSET_LENGTH] ^ m_data[OFFSET_APPTYPE];
  m_appendCS();
}

uint8_t Frame::m_calcCS() const {
  uint8_t cs = START_BYTE;  // start byte not included in checksum

  for (uint8_t data : m_data)
    cs -= data;

  return cs;
}

bool Frame::deserialize(const uint8_t &data) {
  const size_t idx = m_data.size();

  m_data.push_back(data);

  if (idx > OFFSET_DATA) {
    // Header received. Reading data.

    if (idx < m_data[OFFSET_LENGTH])
      return false;

    // Frame received. Return validation result.
    if (m_calcCS() == 0)
      return true;

    m_data.clear();
    return false;
  }

  if (idx > OFFSET_LENGTH)
    return false;

  if (idx == OFFSET_LENGTH && data > OFFSET_DATA)
    return false;

  if (data == START_BYTE)
    return false;

  m_data.clear();
  return false;
}

static char u4hex(uint8_t num) { return num + ((num < 10) ? '0' : ('A' - 10)); }

std::string Frame::toString() const {
  std::string ret;

  if (m_data.empty())
    return ret;

  ret.reserve(3 * m_data.size());

  for (auto it = m_data.begin();;) {
    ret.push_back(u4hex(*it / 16));
    ret.push_back(u4hex(*it % 16));

    if (++it == m_data.end())
      return ret;

    ret.push_back(' ');
  }
}

}  // namespace midea
}  // namespace dudanov
