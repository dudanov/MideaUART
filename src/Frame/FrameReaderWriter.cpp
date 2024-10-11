#include "Frame/FrameReaderWriter.h"
#include "Helpers/Log.h"
#include "IOStream.h"

namespace dudanov {
namespace midea {

static const char *const TAG = "FrameReaderWriter";

inline void FrameReaderWriter::m_updAppID() {
  auto appID = static_cast<ApplianceID>(m_data[IDX_APPLIANCE]);

  if (appID == m_appID)
    return;

  m_appID = appID;

  LOG_D(TAG, "ApplianceID updated to 0x%02X", appID);
}

inline void FrameReaderWriter::m_updProtoID() {
  auto protoID = m_data[IDX_PROTOCOL];

  if (protoID == m_protoID)
    return;

  m_protoID = protoID;

  LOG_D(TAG, "ProtocolID updated to %d", protoID);
}

bool FrameReaderWriter::read() {
  for (uint8_t byte; m_io->read(byte);) {
    const uint8_t idx = m_data.size();

    m_data.push_back(byte);

    if (idx > IDX_LENGTH) {
      // Frame length is known.
      if (idx < m_len())
        continue;

      // Frame received. Return validation result.
      if (m_calcCS() == byte) {
        LOG_D(TAG, "RX: %s", this->toString().c_str());

        m_updAppID();
        m_updProtoID();

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

void FrameReaderWriter::write(FrameType typeID, const FrameData &s) {
  auto frame = Frame(m_appID, m_protoID, typeID, s);

  LOG_D(TAG, "TX: %s", frame.toString().c_str());
  m_io->write(frame.m_data.data(), frame.m_data.size());
}

}  // namespace midea
}  // namespace dudanov
