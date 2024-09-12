#include "Frame/FrameData.h"
#include "Frame/PropertiesReader.h"
#include "Appliance/CapabilitiesBase.h"

namespace dudanov {
namespace midea {

uint8_t CapabilitiesBase::read(const FrameData &s) {
  PropertiesReader r{s};

  for (; r.hasData(); r.advance())
    m_setCapability(r);

  return r.hasHeader() ? r.uuid() : 0;
}

}  // namespace midea
}  // namespace dudanov
