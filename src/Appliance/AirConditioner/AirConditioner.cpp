#include "Appliance/AirConditioner/AirConditioner.h"
#include "Helpers/Timer.h"
#include "Helpers/Log.h"

namespace dudanov {
namespace midea {
namespace ac {

static const char *TAG = "AirConditioner";

void AirConditioner::m_setup() {
  if (m_autoconfStatus != AUTOCONF_DISABLED)
    m_getCapabilities();

  m_timerManager.registerTimer(m_powerUsageTimer);

  m_powerUsageTimer.setCallback([this](Timer *timer) {
    timer->reset();
    m_getPowerUsage();
  });

  m_powerUsageTimer.start(30000);
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
  if (m_sendControl)
    return;

  StatusData status = m_status;
  Mode mode = m_mode;
  Preset preset = m_preset;
  bool hasUpdate = false;
  bool isModeChanged = false;

  if (control.mode.hasUpdate(mode)) {
    hasUpdate = true;
    isModeChanged = true;
    mode = control.mode.value();

    if (m_mode == Mode::MODE_OFF)
      preset = m_lastPreset;

    else if (!checkConstraints(mode, preset))
      preset = Preset::PRESET_NONE;
  }

  if (control.preset.hasUpdate(preset) && checkConstraints(mode, control.preset.value())) {
    hasUpdate = true;
    preset = control.preset.value();
  }

  if (mode != Mode::MODE_OFF) {
    if (mode == Mode::MODE_AUTO || preset != Preset::PRESET_NONE) {
      if (m_fanMode != FanSpeed::FAN_AUTO) {
        hasUpdate = true;
        status.setFanMode(FanSpeed::FAN_AUTO);
      }

    } else if (control.fanMode.hasUpdate(m_fanMode)) {
      hasUpdate = true;
      status.setFanMode(control.fanMode.value());
    }

    if (control.swingMode.hasUpdate(m_swingMode)) {
      hasUpdate = true;
      status.setSwingMode(control.swingMode.value());
    }
  }

  if (control.targetTemp.hasUpdate(m_targetTemp)) {
    hasUpdate = true;
    status.setTargetTemp(control.targetTemp.value());
  }

  if (hasUpdate) {
    m_sendControl = true;
    status.setMode(mode);
    status.setPreset(preset);
    status.setBeeper(m_beeper);
    status.appendCRC();

    if (isModeChanged && preset != Preset::PRESET_NONE && preset != Preset::PRESET_SLEEP) {
      // Last command with preset
      m_setStatus(status);
      status.setPreset(Preset::PRESET_NONE);
      status.setBeeper(false);
      status.updateCRC();

      // First command without preset
      m_queueRequestPriority(FrameType::DEVICE_CONTROL, std::move(status),
                             // onData
                             std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));

    } else {
      m_setStatus(std::move(status));
    }
  }
}

void AirConditioner::m_setStatus(StatusData status) {
  LOG_D(TAG, "Enqueuing a priority SET_STATUS(0x40) request...");

  m_queueRequestPriority(
      FrameType::DEVICE_CONTROL, std::move(status),
      // onData
      std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1),
      // onSuccess
      [this]() { m_sendControl = false; },
      // onError
      [this]() {
        LOG_W(TAG, "SET_STATUS(0x40) request failed...");
        m_sendControl = false;
      });
}

void AirConditioner::setPowerState(bool state) {
  if (state != this->getPowerState()) {
    Control control;
    control.mode = state ? m_status.getRawMode() : Mode::MODE_OFF;
    this->control(control);
  }
}

void AirConditioner::m_getPowerUsage() {
  FrameDataQuery41 data{};
  LOG_D(TAG, "Enqueuing a GET_POWERUSAGE(0x41) request...");

  m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
                 // onData
                 [this](FrameData data) -> ResponseStatus {
                   const auto status = data.to<StatusData>();

                   if (!status.hasPowerInfo())
                     return ResponseStatus::RESPONSE_WRONG;

                   if (m_powerUsage != status.getPowerUsage()) {
                     m_powerUsage = status.getPowerUsage();
                     this->sendUpdate();
                   }

                   return ResponseStatus::RESPONSE_OK;
                 });
}

//
void AirConditioner::m_getCapabilities() {
  if (m_autoconfStatus == AUTOCONF_PROGRESS)
    return;

  FrameDataB5Query b5data;
  m_autoconfStatus = AUTOCONF_PROGRESS;
  LOG_D(TAG, "Enqueuing a priority GET_CAPABILITIES(0xB5) request...");

  m_queueRequest(
      FrameType::DEVICE_QUERY, std::move(b5data),
      // onData
      [this](FrameData data) -> ResponseStatus {
        if (!data.hasID(0xB5))
          return RESPONSE_WRONG;

        const uint8_t next = m_capabilities.read(PropertiesData::fromData(data));

        if (next == 0)
          return RESPONSE_OK;

        FrameDataSecondB5Query b5next(next);
        m_sendFrame(FrameType::DEVICE_QUERY, b5next);

        return RESPONSE_PARTIAL;
      },

      // onSuccess
      [this]() { m_autoconfStatus = AUTOCONF_OK; },

      // onError
      [this]() {
        LOG_W(TAG, "Failed to get 0xB5 capabilities report.");
        m_autoconfStatus = AUTOCONF_ERROR;
      });
}

void AirConditioner::m_getStatus() {
  FrameDataDevQuery41 data;
  LOG_D(TAG, "Enqueuing a GET_STATUS(0x41) request...");

  m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
                 // onData
                 std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
}

void AirConditioner::m_displayToggle() {
  FrameDataLight41 data;
  LOG_D(TAG, "Enqueuing a priority TOGGLE_LIGHT(0x41) request...");

  m_queueRequest(FrameType::DEVICE_QUERY, std::move(data),
                 // onData
                 std::bind(&AirConditioner::m_readStatus, this, std::placeholders::_1));
}

template<typename T> void setProperty(T &property, const T &value, bool &update) {
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
  m_status.copyStatus(newStatus);

  if (m_mode != newStatus.getMode()) {
    hasUpdate = true;
    m_mode = newStatus.getMode();

    if (newStatus.getMode() == Mode::MODE_OFF)
      m_lastPreset = m_preset;
  }
  setProperty(m_preset, newStatus.getPreset(), hasUpdate);
  setProperty(m_fanMode, newStatus.getFanMode(), hasUpdate);
  setProperty(m_swingMode, newStatus.getSwingMode(), hasUpdate);
  setProperty(m_targetTemp, newStatus.getTargetTemp(), hasUpdate);
  setProperty(m_indoorTemp, newStatus.getIndoorTemp(), hasUpdate);
  setProperty(m_outdoorTemp, newStatus.getOutdoorTemp(), hasUpdate);
  setProperty(m_indoorHumidity, newStatus.getHumiditySetpoint(), hasUpdate);

  if (hasUpdate)
    this->sendUpdate();

  return ResponseStatus::RESPONSE_OK;
}

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
