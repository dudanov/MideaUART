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
  StatusData status = this->m_status;
  Mode mode = this->m_mode;
  Preset preset = this->m_preset;
  bool hasUpdate = false;
  if (control.mode.hasUpdate(mode)) {
    hasUpdate = true;
    mode = control.mode.value();
    if (!checkConstraints(mode, preset))
      preset = Preset::PRESET_NONE;
  }
  if (control.preset.hasUpdate(preset) && checkConstraints(mode, control.preset.value())) {
    hasUpdate = true;
    preset = control.preset.value();
  }
  if (mode != Mode::MODE_OFF) {
    if (mode == Mode::MODE_AUTO || preset != Preset::PRESET_NONE) {
      if (this->m_fanMode != FanMode::FAN_AUTO) {
        status.setFanMode(FanMode::FAN_AUTO);
        hasUpdate = true;
      }
    } else if (control.fanMode.hasUpdate(this->m_fanMode)) {
      status.setFanMode(control.fanMode.value());
      hasUpdate = true;
    }
    if (control.swingMode.hasUpdate(this->m_swingMode)) {
      status.setSwingMode(control.swingMode.value());
      hasUpdate = true;
    }
  }
  if (control.targetTemp.hasUpdate(this->m_targetTemp)) {
    hasUpdate = true;
    status.setTargetTemp(control.targetTemp.value());
  }
  if (hasUpdate) {
    status.setMode(mode);
    status.setPreset(preset);
    status.setBeeper(this->m_beeper);
    status.appendCRC();
    LOG_D(TAG, "Enqueuing a priority SET_STATUS(0x40) request...");
    this->m_queueRequestPriority(FrameType::DEVICE_CONTROL, std::move(status),
                    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
  }
}

void AirConditioner::m_getPowerUsage() {
  QueryPowerData data{};
  LOG_D(TAG, "Enqueuing a GET_POWERUSAGE(0x41) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data), [this](FrameData data) -> ResponseStatus {
    const auto status = data.to<StatusData>();
    if (!status.hasPowerInfo())
      return ResponseStatus::RESPONSE_WRONG;
    if (this->m_powerUsage != status.getPowerUsage()) {
      this->m_powerUsage = status.getPowerUsage();
      this->sendUpdate();
    }
    return ResponseStatus::RESPONSE_OK;
  });
}

void AirConditioner::m_getCapabilities() {
  GetCapabilitiesData data{};
  this->m_autoconfStatus = AUTOCONF_PROGRESS;
  LOG_D(TAG, "Enqueuing a priority GET_CAPABILITIES(0xB5) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data), [this](FrameData data) -> ResponseStatus {
    if (!data.hasID(0xB5))
      return ResponseStatus::RESPONSE_WRONG;
    if (this->m_capabilities.read(data)) {
      GetCapabilitiesSecondData data{};
      this->m_sendFrame(FrameType::DEVICE_QUERY, data);
      return ResponseStatus::RESPONSE_PARTIAL;
    }
    this->m_autoconfStatus = AUTOCONF_OK;
    return ResponseStatus::RESPONSE_OK;
  },
  [this]() {
    LOG_W(TAG, "Failed to get 0xB5 capabilities report.");
    this->m_autoconfStatus = AUTOCONF_ERROR;
  });
}

void AirConditioner::m_getStatus() {
  QueryStateData data{};
  LOG_D(TAG, "Enqueuing a GET_STATUS(0x41) request...");
  this->m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
                    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
}

void AirConditioner::m_displayToggle() {
  DisplayToggleData data{};
  LOG_D(TAG, "Enqueuing a priority TOGGLE_LIGHT(0x41) request...");
  this->m_queueRequestPriority(FrameType::DEVICE_QUERY, std::move(data),
                    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
}

template<typename T>
void setProperty(T &property, const T &value, bool &update) {
  if (property != value) {
    property = value;
    update = true;
  }
}

ResponseStatus AirConditioner::m_readStatus(FrameData data) {
  if (!data.hasStatus())
    return ResponseStatus::RESPONSE_WRONG;
  LOG_D(TAG, "New status data received. Parsing...");
  bool hasUpdate = false;
  const StatusData newStatus = data.to<StatusData>();
  this->m_status.copyStatus(newStatus);
  setProperty(this->m_mode, newStatus.getMode(), hasUpdate);
  setProperty(this->m_preset, newStatus.getPreset(), hasUpdate);
  setProperty(this->m_fanMode, newStatus.getFanMode(), hasUpdate);
  setProperty(this->m_swingMode, newStatus.getSwingMode(), hasUpdate);
  setProperty(this->m_targetTemp, newStatus.getTargetTemp(), hasUpdate);
  setProperty(this->m_indoorTemp, newStatus.getIndoorTemp(), hasUpdate);
  setProperty(this->m_outdoorTemp, newStatus.getOutdoorTemp(), hasUpdate);
  setProperty(this->m_indoorHumidity, newStatus.getHumiditySetpoint(), hasUpdate);
  if (hasUpdate)
    this->sendUpdate();
  return ResponseStatus::RESPONSE_OK;
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
