#include "Frame/PropertyQuery.h"

namespace dudanov {
namespace midea {

void PropertyQuery::getProperty(PropertyUUID uuid) {
  this->append(uuid);
  this->m_data[1]++;  // increment number of commands
}

PropertyQuery::ResponseReader PropertyQuery::getReader() const { return ResponseReader(*this); }

}  // namespace midea
}  // namespace dudanov
