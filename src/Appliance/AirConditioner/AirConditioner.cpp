#include "Appliance/AirConditioner/AirConditioner.h"
#include "Helpers/Timer.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "AirConditioner";

void AirConditioner::m_setup() {
  if (this->m_autoconfStatus != AUTOCONF_DISABLED)
    this->m_getCapabilities();
  this->m_timerManager.registerTimer(this->m_powerUsageTimer);
  this->m_powerUsageTimer.setCallback([this](Timer *timer) {
    timer->reset();
    this->m_getPowerUsage();
  });
  this->m_powerUsageTimer.start(30000);
}

static bool checkConstraints(const Mode &mode, const Preset &preset) {
  if (mode == Mode::MODE_OFF)
    return preset == Preset::PRESET_NONE;
  switch (preset) {
    case Preset::PRESET_NONE:
      return true;
    case Preset::PRESET_ECO:
      return mode == Mode::MODE_COOL;
    case Preset::PRESET_TURBO:
      return mode == Mode::MODE_COOL || mode == Mode::MODE_HEAT;
    case Preset::PRESET_SLEEP:
      return mode != Mode::MODE_DRY && mode != Mode::MODE_FAN_ONLY;
    case Preset::PRESET_FREEZE_PROTECTION:
      return mode == Mode::MODE_HEAT;
    default:
      return false;
  }
}

void AirConditioner::control(const Control &control) {
  if (this->m_sendControl)
    return;
  StatusData status = this->m_status;
  Mode mode = this->m_mode;
  Preset preset = this->m_preset;
  bool hasUpdate = false;
  bool isModeChanged = false;
  if (control.mode.hasUpdate(mode)) {
    hasUpdate = true;
    isModeChanged = true;
    mode = control.mode.value();
    if (this->m_mode == Mode::MODE_OFF)
      preset = this->m_lastPreset;
    else if (!checkConstraints(mode, preset))
      preset = Preset::PRESET_NONE;
  }
  if (control.preset.hasUpdate(preset) && checkConstraints(mode, control.preset.value())) {
    hasUpdate = true;
    preset = control.preset.value();
  }
  if (mode != Mode::MODE_OFF) {
    if (mode == Mode::MODE_AUTO || preset != Preset::PRESET_NONE) {
      if (this->m_fanMode != FanMode::FAN_AUTO) {
        hasUpdate = true;
        status.setFanMode(FanMode::FAN_AUTO);
      }
    } else if (control.fanMode.hasUpdate(this->m_fanMode)) {
      hasUpdate = true;
      status.setFanMode(control.fanMode.value());
    }
    if (control.swingMode.hasUpdate(this->m_swingMode)) {
      hasUpdate = true;
      status.setSwingMode(control.swingMode.value());
    }
  }
  if (control.targetTemp.hasUpdate(this->m_targetTemp)) {
    hasUpdate = true;
    status.setTargetTemp(control.targetTemp.value());
  }
  if (hasUpdate) {
    this->m_sendControl = true;
    status.setMode(mode);
    status.setPreset(preset);
    status.setBeeper(this->m_beeper);
    status.appendCRC();
    if (isModeChanged && preset != Preset::PRESET_NONE && preset != Preset::PRESET_SLEEP) {
      // Last command with preset
      this->m_setStatus(status);
      status.setPreset(Preset::PRESET_NONE);
      status.setBeeper(false);
      status.updateCRC();
      // First command without preset
      this->m_queueRequestPriority(FrameType::DEVICE_CONTROL, std::move(status),
        // onData
        std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1)
      );
    } else {
      this->m_setStatus(std::move(status));
    }
  }
}

bool AirConditioner::hasFreshStatus(uint32_t maxAge) const {
  return this->m_hasStatus && maxAge &&
    static_cast<uint32_t>(millis() - this->m_statusReceivedAt) <= maxAge;
}

bool AirConditioner::setPurifier(bool state, uint32_t maxAge) {
  if (this->m_sendControl || !this->hasFreshStatus(maxAge) || state == this->m_purifier)
    return false;
  // Do not use control(): its preset/mode constraints can coerce the raw fan.
  StatusData status = this->m_purifierStatus;
  status.setPurifier(state);
  status.setBeeper(this->m_beeper);
  status.appendCRC();
  this->m_sendControl = true;
  const uint32_t revision = this->m_statusRevision;
  this->m_setStatus(std::move(status), [this, maxAge, revision]() {
    return this->hasFreshStatus(maxAge) && this->m_statusRevision == revision;
  });
  return true;
}

void AirConditioner::m_setStatus(StatusData status, RequestGuard canSend) {
  LOG_D(TAG, "Enqueuing a priority SET_STATUS(0x40) request...");
  this->m_queueRequestPriority(FrameType::DEVICE_CONTROL, std::move(status),
    // onData
    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1),
    // onSuccess
    [this]() {
      this->m_sendControl = false;
    },
    // onError
    [this]() {
      LOG_W(TAG, "SET_STATUS(0x40) request failed...");
      this->m_sendControl = false;
    },
    std::move(canSend)
  );
}

void AirConditioner::setPowerState(bool state) {
  if (state != this->getPowerState()) {
    Control control;
    control.mode = state ? this->m_status.getRawMode() : Mode::MODE_OFF;
    this->control(control);
  }
}

void AirConditioner::m_getPowerUsage() {
  QueryPowerData data{};
  LOG_D(TAG, "Enqueuing a GET_POWERUSAGE(0x41) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
    // onData
    [this](FrameData data) -> ResponseStatus {
      const auto status = data.to<StatusData>();
      if (!status.hasPowerInfo())
        return ResponseStatus::RESPONSE_WRONG;
      if (this->m_powerUsage != status.getPowerUsage()) {
        this->m_powerUsage = status.getPowerUsage();
        this->sendUpdate();
      }
      return ResponseStatus::RESPONSE_OK;
    }
  );
}

void AirConditioner::m_getCapabilities() {
  GetCapabilitiesData data{};
  this->m_autoconfStatus = AUTOCONF_PROGRESS;
  LOG_D(TAG, "Enqueuing a priority GET_CAPABILITIES(0xB5) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
    // onData
    [this](FrameData data) -> ResponseStatus {
      if (!data.hasID(0xB5))
        return ResponseStatus::RESPONSE_WRONG;
      if (this->m_capabilities.read(data)) {
        GetCapabilitiesSecondData data{};
        this->m_sendFrame(FrameType::DEVICE_QUERY, data);
        return ResponseStatus::RESPONSE_PARTIAL;
      }
      return ResponseStatus::RESPONSE_OK;
    },
    // onSuccess
    [this]() {
      this->m_autoconfStatus = AUTOCONF_OK;
    },
    // onError
    [this]() {
      LOG_W(TAG, "Failed to get 0xB5 capabilities report.");
      this->m_autoconfStatus = AUTOCONF_ERROR;
    }
  );
}

void AirConditioner::m_getStatus() {
  QueryStateData data{};
  LOG_D(TAG, "Enqueuing a GET_STATUS(0x41) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
    // onData
    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1)
  );
}

void AirConditioner::m_displayToggle() {
  DisplayToggleData data{};
  LOG_D(TAG, "Enqueuing a priority TOGGLE_LIGHT(0x41) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
    // onData
    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1)
  );
}

template<typename T>
void setProperty(T &property, const T &value, bool &update) {
  if (property != value) {
    property = value;
    update = true;
  }
}

// Feature-local integrity gate, not a replacement for the climate parser.
// Aligns with the formats proposed in #50: 16-byte prefix plus CRC8 or
// legacy additive checksum. Global validation remains a separate change.
static bool hasTrustedPurifierStatus(const FrameData &data) {
  if (data.size() < 17)
    return false;
  if (data.hasValidCRC())
    return true;
  uint8_t sum = 0;
  for (unsigned i = 0; i < data.size(); i++)
    sum += data.data()[i];
  return sum == 0;
}

ResponseStatus AirConditioner::m_readStatus(FrameData data) {
  // Busy/ACK/nonstatus bodies cannot revoke or supersede a confirmed snapshot.
  // Guard the identifier lookup too: FrameData::hasStatus() requires byte zero.
  if (!data.size() || !data.hasStatus())
    return ResponseStatus::RESPONSE_WRONG;
  // copyStatus() needs ten bytes after the identifier. Avoid its existing
  // unchecked memcpy on truncated C0 inputs, without imposing a new full length.
  if (data.size() < 11) {
    this->m_hasStatus = false;
    return ResponseStatus::RESPONSE_WRONG;
  }
  ++this->m_statusRevision;
  this->m_hasStatus = hasTrustedPurifierStatus(data);
  if (this->m_hasStatus) {
    // Strip integrity byte before reading optional fields in shorter replies.
    const StatusData confirmed(FrameData(data.data(), data.size() - 1));
    // Build from the SET template, never echo C0 reserved/response bits.
    const StatusData commandTemplate;
    std::vector<uint8_t> command(commandTemplate.data(), commandTemplate.data() + commandTemplate.size());
    command[1] = confirmed.data()[1] & 0x01;  // power, including OFF
    command[2] = confirmed.data()[2] & 0xE0;  // raw mode, even while OFF
    command[3] = confirmed.data()[3];         // raw fan: do not normalize 30/50
    const bool turbo = (confirmed.data()[8] & 0x20) || (confirmed.data()[10] & 0x02);
    command[8] = turbo ? 0x20 : 0;
    command[9] = (confirmed.data()[9] & 0x10) ? 0x80 : 0;  // ECO status -> SET
    command[10] = (confirmed.data()[10] & 0x05) | (turbo ? 0x02 : 0);  // SLEEP, units, TURBO
    if (confirmed.size() > 21)
      command[21] = confirmed.data()[21] & 0x80;  // freeze protection is optional
    this->m_purifierStatus = StatusData(FrameData(command.data(), command.size()));
    this->m_purifierStatus.setSwingMode(confirmed.getSwingMode());
    this->m_purifierStatus.setTargetTemp(confirmed.getTargetTemp());
    this->m_purifierStatus.setPurifier(confirmed.getPurifier());
    this->m_statusReceivedAt = millis();
    this->m_purifier = confirmed.getPurifier();
  }
  LOG_D(TAG, "New status data received. Parsing...");
  bool hasUpdate = false;
  // Climate getters must not interpret the trailing integrity byte as an
  // optional field either. Keep the legacy ten-byte command-cache copy from
  // the original C0 prefix (also safe for the existing 11-byte minimum).
  const StatusData newStatus(FrameData(data.data(), data.size() - 1));
  this->m_status.copyStatus(StatusData(data));
  if (this->m_mode != newStatus.getMode()) {
    hasUpdate = true;
    this->m_mode = newStatus.getMode();
    if (newStatus.getMode() == Mode::MODE_OFF)
      this->m_lastPreset = this->m_preset;
  }
  setProperty(this->m_preset, newStatus.getPreset(), hasUpdate);
  setProperty(this->m_fanMode, newStatus.getFanMode(), hasUpdate);
  setProperty(this->m_swingMode, newStatus.getSwingMode(), hasUpdate);
  setProperty(this->m_targetTemp, newStatus.getTargetTemp(), hasUpdate);
  setProperty(this->m_indoorTemp, newStatus.getIndoorTemp(), hasUpdate);
  setProperty(this->m_outdoorTemp, newStatus.getOutdoorTemp(), hasUpdate);
  setProperty(this->m_indoorHumidity, newStatus.getHumiditySetpoint(), hasUpdate);
  if (hasUpdate)
    this->sendUpdate();
  if (this->m_hasStatus)
    for (auto &cb : this->m_purifierCallbacks)
      cb(this->m_purifier);
  return ResponseStatus::RESPONSE_OK;
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
