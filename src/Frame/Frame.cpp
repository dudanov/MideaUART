#include "Frame/Frame.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {

static const char *const TAG = "Frame";

Frame::Frame(ApplianceID applianceID, uint8_t protocolID, FrameType typeID, const FrameData &s)
    : m_data{{SYM_START, 0, applianceID, 0, 0, 0, 0, 0, protocolID, typeID}} {
  this->setData(s);
}

FrameData Frame::getData() const { return FrameData{&m_data[IDX_DATA], &m_data[m_len()]}; }

void Frame::setData(const FrameData &s) {
  const uint8_t new_size = s.m_data.size() + IDX_DATA;

  m_data.resize(new_size + 1);  // +1 byte for the checksum

  m_data[IDX_LENGTH] = new_size;
  m_data[IDX_SYNC] = new_size ^ m_data[IDX_APPLIANCE];

  *std::copy(s.m_data.begin(), s.m_data.end(), m_data.begin() + IDX_DATA) = m_calcCS();
}

uint8_t Frame::m_calcCS() const {
  uint8_t cs{};
  const auto it{&m_data[IDX_LENGTH]};

  std::for_each(it, &m_data[*it], [&](auto x) { cs -= x; });
  return cs;
}

bool Frame::deserialize(const uint8_t &data) {
  const uint8_t idx = m_data.size();

  m_data.push_back(data);

  if (idx > IDX_LENGTH) {
    // Frame length is known.
    if (idx < m_len())
      return false;

    // Frame received. Return validation result.
    if (m_calcCS() == data)
      return true;

    LOG_W(TAG, "Checksum is wrong.");

  } else if ((idx == IDX_LENGTH && data > IDX_DATA) || data == SYM_START) {
    return false;
  }

  m_data.clear();
  return false;
}

static char u4hex(uint8_t x) { return x + (x < 10 ? '0' : '7'); }

std::string Frame::toString() const {
  std::string str{};

  if (!m_data.empty()) {
    str.assign(3 * m_data.size() - 1, ' ');
    auto dst{str.begin()};

    for (auto it{m_data.begin()}; it != m_data.end(); dst += 3, ++it) {
      dst[0] = u4hex(*it / 16);
      dst[1] = u4hex(*it % 16);
    }
  }

  return str;
}

}  // namespace midea
}  // namespace dudanov
