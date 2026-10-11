#pragma once
#include "Helpers/Platform.h"
#include "Appliance/ApplianceBase.h"
#include "Appliance/AirConditioner/Capabilities.h"
#include "Appliance/AirConditioner/StatusData.h"
#include "Helpers/Helpers.h"

namespace dudanov {
namespace midea {
namespace ac {

using OnPurifierStatusCallback = std::function<void(bool)>;

// Air conditioner control command
struct Control {
  Optional<float> targetTemp{};
  Optional<Mode> mode{};
  Optional<Preset> preset{};
  Optional<FanMode> fanMode{};
  Optional<SwingMode> swingMode{};
};

class AirConditioner : public ApplianceBase {
 public:
  AirConditioner() : ApplianceBase(AIR_CONDITIONER) {}
  void m_setup() override;
  void m_onIdle() override { this->m_getStatus(); }
  void control(const Control &control);
  void setPowerState(bool state);
  bool getPowerState() const { return this->m_mode != Mode::MODE_OFF; }
  void togglePowerState() { this->setPowerState(this->m_mode == Mode::MODE_OFF); }
  float getTargetTemp() const { return this->m_targetTemp; }
  float getIndoorTemp() const { return this->m_indoorTemp; }
  float getOutdoorTemp() const { return this->m_outdoorTemp; }
  float getIndoorHum() const { return this->m_indoorHumidity; }
  float getPowerUsage() const { return this->m_powerUsage; }
  Mode getMode() const { return this->m_mode; }
  SwingMode getSwingMode() const { return this->m_swingMode; }
  FanMode getFanMode() const { return this->m_fanMode; }
  Preset getPreset() const { return this->m_preset; }
  const Capabilities &getCapabilities() const { return this->m_capabilities; }
  void displayToggle() { this->m_displayToggle(); }
  /// Last purifier value confirmed by a status response (initially false).
  bool getPurifier() const { return this->m_purifier; }
  /// True only when queued; false if unknown/stale, busy, or already at this value.
  /// State changes only on trustworthy appliance feedback, never on this request.
  bool setPurifier(bool state, uint32_t maxAge = 15000);
  /// maxAge is in milliseconds; zero disables freshness (and purifier writes).
  bool hasFreshStatus(uint32_t maxAge = 15000) const;
  /// Called for every accepted status, including unchanged/rejected set responses.
  void addOnPurifierStatusCallback(OnPurifierStatusCallback cb) { this->m_purifierCallbacks.push_back(cb); }
 protected:
  void m_getPowerUsage();
  void m_getCapabilities();
  void m_getStatus();
  void m_setStatus(StatusData status, RequestGuard canSend = nullptr);
  void m_displayToggle();
  ResponseStatus m_readStatus(FrameData data);
  Capabilities m_capabilities{};
  Timer m_powerUsageTimer;
  float m_indoorHumidity{};
  float m_indoorTemp{};
  float m_outdoorTemp{};
  float m_targetTemp{};
  float m_powerUsage{};
  Mode m_mode{Mode::MODE_OFF};
  Preset m_preset{Preset::PRESET_NONE};
  FanMode m_fanMode{FanMode::FAN_AUTO};
  SwingMode m_swingMode{SwingMode::SWING_OFF};
  Preset m_lastPreset{Preset::PRESET_NONE};
  StatusData m_status{};
  bool m_sendControl{};
  bool m_purifier{};
  // Separate trusted command snapshot: malformed climate replies must not seed writes.
  StatusData m_purifierStatus{};
  bool m_hasStatus{};
  uint32_t m_statusReceivedAt{};
  uint32_t m_statusRevision{};
  std::vector<OnPurifierStatusCallback> m_purifierCallbacks;
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
