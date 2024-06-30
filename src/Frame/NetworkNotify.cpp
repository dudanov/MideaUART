#include "Frame/NetworkNotify.h"
#include <IPAddress.h>

namespace dudanov {
namespace midea {

void NetworkNotifyData::setIP(const IPAddress &ip) {
  this->m_data[3] = ip[3];
  this->m_data[4] = ip[2];
  this->m_data[5] = ip[1];
  this->m_data[6] = ip[0];
}

}  // namespace midea
}  // namespace dudanov
