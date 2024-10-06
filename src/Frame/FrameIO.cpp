#include "Frame/FrameIO.h"
#include "Helpers/Log.h"

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
        LOG_D(TAG, "RX: %s", this->toString().c_str());
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
