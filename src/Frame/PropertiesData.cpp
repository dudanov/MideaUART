#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {

PropertiesReader::PropertiesReader(const FrameData &s)
    : m_pheader{&s[2]}, m_pdata{&s[5] + !s.hasID(0xB5)}, m_pend{&s[-1]} {}

bool PropertiesReader::hasProperties(const FrameData &s) { return s.hasID(0xB0) || s.hasID(0xB1) || s.hasID(0xB5); }

void PropertiesData::appendUUID(PropertyUUID uuid) {
  this->append(uuid);
  m_data[1]++;  // increment number of UUIDs
}

}  // namespace midea
}  // namespace dudanov
