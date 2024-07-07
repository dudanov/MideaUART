#include "Frame/PropertyQuery.h"

namespace dudanov {
namespace midea {

void PropertyQuery::appendUUID(PropertyUUID uuid) {
  this->append(uuid);
  this->m_data[1]++;  // increment number of UUIDs
}

}  // namespace midea
}  // namespace dudanov
