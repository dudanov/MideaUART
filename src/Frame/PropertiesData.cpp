#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {

void PropertiesData::appendUUID(PropertyUUID uuid) {
  this->append(uuid);
  m_data[1]++;  // increment number of UUIDs
}

PropertiesData::PropertiesReader PropertiesData::getReader() const { return PropertiesReader{*this}; }

}  // namespace midea
}  // namespace dudanov
