#include "Frame/FrameReader.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {

static const char *const TAG = "FrameReader";

bool FrameReader::deserialize(const uint8_t &data) {
  const uint8_t idx = m_data.size();

  m_data.push_back(data);

  if (idx > IDX_LENGTH) {
    // Frame length is known.
    if (idx < m_len())
      return false;

    // Frame received. Return validation result.
    if (m_calcCS() == data)
      return true;

    LOG_W(TAG, "Checksum is wrong.");

  } else if ((idx == IDX_LENGTH && data > IDX_DATA) || data == SYM_START) {
    return false;
  }

  m_data.clear();
  return false;
}

}  // namespace midea
}  // namespace dudanov
