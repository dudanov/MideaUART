#include "Frame/PropertiesReader.h"

namespace dudanov {
namespace midea {

PropertiesReader::PropertiesReader(const FrameData &s)
    : m_pheader{&s[2]}, m_pdata{&s[5] + !s.hasID(0xB5)}, m_pend{&s[-1]} {}

void PropertiesReader::advance() {
  auto offset = m_size();
  m_pheader += offset;
  m_pdata += offset;
}

}  // namespace midea
}  // namespace dudanov
