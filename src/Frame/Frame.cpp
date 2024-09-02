#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

Frame::Frame(uint8_t applianceID, uint8_t protocolID, uint8_t typeID, const FrameData &data)
    : m_data({START_BYTE, 0x00, applianceID, 0x00, 0x00, 0x00, 0x00, 0x00, protocolID, typeID}) {
  this->setData(data);
}

FrameData Frame::getData() const { return FrameData(&m_data[OFFSET_DATA], &m_data[m_len()]); }

void Frame::setData(const FrameData &data) {
  m_trimData();
  m_appendData(data);
  m_data[OFFSET_LENGTH] = m_data.size();
  m_appendCS();
}

uint8_t Frame::m_calcCS() const {
  // Start byte excluded from checksum.
  uint8_t cs = START_BYTE;

  for (uint8_t data : m_data)
    cs -= data;

  return cs;
}

bool Frame::deserialize(const uint8_t &data) {
  const uint8_t idx = m_data.size();

  m_data.push_back(data);

  if (idx > OFFSET_LENGTH) {
    // Frame length is known.
    if (idx < m_len())
      return false;

    // Frame received. Return validation result.
    if (m_calcCS() == 0)
      return true;

  } else if ((idx == OFFSET_LENGTH && data > OFFSET_DATA) || data == START_BYTE) {
    return false;
  }

  m_data.clear();
  return false;
}

static char u4hex(uint8_t num) { return num + ((num < 10) ? '0' : ('A' - 10)); }

std::string Frame::toString() const {
  std::string str;

  if (!m_data.empty()) {
    str.assign(3 * m_data.size() - 1, ' ');
    auto dst = str.begin();

    for (auto it = m_data.begin(); it != m_data.end(); dst += 3, ++it) {
      dst[0] = u4hex(*it / 16);
      dst[1] = u4hex(*it % 16);
    }
  }

  return str;
}

}  // namespace midea
}  // namespace dudanov
