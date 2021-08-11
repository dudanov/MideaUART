#include "Appliance/AirConditioner/AirConditioner.h"
#include "Helpers/Timer.h"

namespace dudanov {
namespace midea {
namespace ac {

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

void AirConditioner::control(Control &control) {
  StatusData status = this->m_status;
  bool needUpdate = false;
  if (control.mode.hasUpdate(status.getMode())) {
    needUpdate = true;
    status.setMode(control.mode.value());
    if (!checkConstraints(control.mode.value(), status.getPreset()))
      status.setPreset(Preset::PRESET_NONE);
  }
  if (control.preset.hasUpdate(status.getPreset())
          && checkConstraints(status.getMode(), control.preset.value())) {
    status.setPreset(control.preset.value());
    needUpdate = true;
  }
  if (control.targetTemp.hasUpdate(status.getTargetTemp())) {
    status.setTargetTemp(control.targetTemp.value());
    needUpdate = true;
  }
  if (control.swingMode.hasUpdate(status.getSwingMode())) {
    status.setSwingMode(control.swingMode.value());
    needUpdate = true;
  }
  if (status.getMode() == Mode::MODE_AUTO)
    control.fanMode = FanMode::FAN_AUTO;
  if (control.fanMode.hasUpdate(status.getFanMode())) {
    status.setFanMode(control.fanMode.value());
    needUpdate = true;
  }
  if (needUpdate) {
    status.setBeeper(this->m_beeper);
    status.appendCRC();
    this->m_queueRequestPriority(FrameType::DEVICE_CONTROL, status,
                    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
  }
}

void AirConditioner::m_getPowerUsage() {
  auto data = QueryPowerData();
  this->m_queueRequest(FrameType::DEVICE_QUERY, data, [this](FrameData data) -> ResponseStatus {
    const auto status = data.as<StatusData>();
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
  auto data = GetCapabilitiesData();
  this->m_autoconfStatus = AUTOCONF_PROGRESS;
  this->m_queueRequest(FrameType::DEVICE_QUERY, data, [this](FrameData data) -> ResponseStatus {
    if (!data.hasID(0xB5))
      return ResponseStatus::RESPONSE_WRONG;
    if (this->m_capabilities.read(data)) {
      auto data = GetCapabilitiesSecondData();
      this->m_sendFrame(FrameType::DEVICE_QUERY, data);
      return ResponseStatus::RESPONSE_PARTIAL;
    }
    this->m_autoconfStatus = AUTOCONF_OK;
    return ResponseStatus::RESPONSE_OK;
  },
  [this]() {
    this->m_autoconfStatus = AUTOCONF_ERROR;
  });
}

void AirConditioner::m_getStatus() {
  auto data = QueryStateData();
  this->m_queueRequest(FrameType::DEVICE_QUERY, data,
                    std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
}

void AirConditioner::m_displayToggle() {
  auto data = DisplayToggleData();
  this->m_queueRequestPriority(FrameType::DEVICE_QUERY, data,
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
  const auto newStatus = data.as<StatusData>();
  if (!newStatus.hasStatus())
    return ResponseStatus::RESPONSE_WRONG;
  this->m_status.copyStatus(newStatus);
  bool needUpdate = false;
  setProperty(this->m_mode, newStatus.getMode(), needUpdate);
  setProperty(this->m_preset, newStatus.getPreset(), needUpdate);
  setProperty(this->m_fanMode, newStatus.getFanMode(), needUpdate);
  setProperty(this->m_swingMode, newStatus.getSwingMode(), needUpdate);
  setProperty(this->m_targetTemp, newStatus.getTargetTemp(), needUpdate);
  setProperty(this->m_indoorTemp, newStatus.getIndoorTemp(), needUpdate);
  setProperty(this->m_outdoorTemp, newStatus.getOutdoorTemp(), needUpdate);
  setProperty(this->m_indoorHumidity, newStatus.getHumiditySetpoint(), needUpdate);
  if (needUpdate)
    this->sendUpdate();
  return ResponseStatus::RESPONSE_OK;
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
