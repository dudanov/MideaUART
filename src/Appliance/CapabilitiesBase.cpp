#include "Appliance/CapabilitiesBase.h"
#include "Frame/FrameData.h"
#include "Frame/PropertiesReader.h"

namespace dudanov {
namespace midea {

uint8_t CapabilitiesBase::update(const FrameData &s) {
  PropertiesReader reader{s};

  for (; reader.hasData(); reader.advance())
    m_setCapability(reader);

  return reader.hasHeader() ? reader.uuid() : 0;
}

}  // namespace midea
}  // namespace dudanov
