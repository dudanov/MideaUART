#include <algorithm>
#include "Frame/Frame.h"

namespace dudanov {
namespace midea {

FrameData Frame::data() const { return {&m_data[IDX_DATA], &m_data[m_len()]}; }

Frame::Frame(ApplianceID applianceID, uint8_t protocolID, FrameType typeID, const FrameData &s) {
  const uint8_t len = s.m_data.size() + IDX_DATA;

  m_data.resize(len + 1);  // +1 byte for the checksum

  m_data[IDX_START] = SYM_START;
  m_data[IDX_LENGTH] = len;
  m_data[IDX_APPLIANCE] = applianceID;
  m_data[IDX_SYNC] = len ^ applianceID;
  m_data[IDX_PROTOCOL] = protocolID;
  m_data[IDX_TYPE] = typeID;

  const auto cs = std::copy(s.m_data.begin(), s.m_data.end(), m_data.begin() + IDX_DATA);
  *cs = m_calcCS();
}

uint8_t Frame::m_calcCS() const {
  uint8_t cs{};
  const auto it{&m_data[IDX_LENGTH]};

  std::for_each(it, &m_data[*it], [&](auto x) { cs -= x; });
  return cs;
}

static char u4hex(uint8_t x) { return x + (x < 10 ? '0' : '7'); }

std::string Frame::toString(const char sep) const {
  if (m_data.empty())
    return {};

  std::string str(3 * m_data.size() - 1, sep);
  auto dst{str.begin()};

  for (auto it{m_data.begin()}; it != m_data.end(); dst += 3, ++it) {
    dst[0] = u4hex(*it / 16);
    dst[1] = u4hex(*it % 16);
  }

  return str;
}

}  // namespace midea
}  // namespace dudanov
