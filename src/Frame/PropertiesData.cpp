#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {

void PropertiesData::appendUUID(PropertyUUID uuid) {
  this->append(uuid);
  m_data[1]++;  // increment number of UUIDs
}

PropertiesData::PropertiesReader PropertiesData::getReader() const {
  size_t header_length = 3;

  if (this->hasID(0xB0) || this->hasID(0xB1))
    header_length = 4;

  return PropertiesReader{*this, header_length};
}

}  // namespace midea
}  // namespace dudanov
