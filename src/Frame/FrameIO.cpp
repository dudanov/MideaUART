#include "Frame/FrameIO.h"
#include "Helpers/Log.h"
#include "IOStream.h"

namespace dudanov {
namespace midea {

static const char *const TAG = "FrameIO";

bool FrameIO::read() {
  for (uint8_t byte; m_io->read(byte);) {
    const uint8_t idx = m_data.size();

    m_data.push_back(byte);

    if (idx > IDX_LENGTH) {
      // Frame length is known.
      if (idx < m_len())
        continue;

      // Frame received. Return validation result.
      if (m_calcCS() == byte) {
        auto appID = static_cast<ApplianceID>(m_data[IDX_APPLIANCE]);
        auto protoID = m_data[IDX_PROTOCOL];

        LOG_D(TAG, "RX: %s", this->toString().c_str());

        if (appID != m_applianceID)
          LOG_D(TAG, "ApplianceID updated to 0x%02X", appID);

        if (protoID != m_protocolID)
          LOG_D(TAG, "ProtocolID updated to %d", appID);

        m_applianceID = appID;
        m_protocolID = protoID;

        return true;
      }

      LOG_W(TAG, "Checksum is wrong.");

    } else if ((idx == IDX_LENGTH && byte > IDX_DATA) || byte == SYM_START) {
      continue;
    }

    m_data.clear();
  }

  return false;
}

void FrameIO::write(const Frame &frame) {
  LOG_D(TAG, "TX: %s", frame.toString().c_str());
  m_io->write(frame.m_data.data(), frame.m_data.size());
}

}  // namespace midea
}  // namespace dudanov
