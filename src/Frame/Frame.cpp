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
  const size_t len = m_data.size();

  if (len > OFFSET_DATA) {
    const size_t elen = m_data[OFFSET_LENGTH];

    if (len > elen) {
      // New frame receiving. Clear data and restart deserializer.
      m_data.clear();
      return this->deserialize(data);
    }

    m_data.push_back(data);

    if (len < elen)
      return false;

    // Frame received. Return validation result.
    return this->isValid();
  }

  if (len == OFFSET_START) {
    // Expected frame start byte.
    if (data != START_BYTE)
      return false;

  } else if (len == OFFSET_LENGTH) {
    // Length received.

    if (data <= OFFSET_DATA) {
      // Length can't be less than header length. Clear frame.
      m_data.clear();
      return false;
    }

    m_data.reserve(data + 1);
  }

  m_data.push_back(data);
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
