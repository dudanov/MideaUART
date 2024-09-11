#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {

void PropertiesData::appendUUID(PropertyUUID uuid) {
  this->append(uuid);
  m_data[1]++;  // increment number of UUIDs
}

}  // namespace midea
}  // namespace dudanov
