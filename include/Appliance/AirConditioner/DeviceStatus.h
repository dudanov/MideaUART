#pragma once
#include <Arduino.h>
#include "Frame/FrameData.h"
#include "StatusData.h"

namespace dudanov {
namespace midea {
namespace ac {

class DeviceStatus {
  friend class FrameStatusData;

 public:
  DeviceStatus() = default;
  DeviceStatus(const DeviceStatus &deviceStatus) = default;

  float getTargetTemperature() const;
  void setTargetTemperature(float value);
  FanSpeed getFanMode() const;
  void setFanSpeed(FanSpeed value) { fanSpeed = value; }
  void setFanSpeed(uint8_t value) { fanSpeed = value; }

 protected:
  /// Wind Off Me. Only in `COOL` and `HEAT`. Turn OFF all Swing.
  bool avoidPeople{};
  /// Wind On Me. Only in `COOL` and `HEAT`. Turn ON all Swing.
  bool blowingPeople{};
  /// Breeze Away.
  bool noWindOnMe{};

  /// Beeper Feedback. Only in Command.
  bool beeper{};
  /// Frost Protection mode (8 Degrees Heating).
  bool Eight_Hot{};

  /// Reset Air Filter Maintenance Timer. Only in Command.
  bool cleanFanTime{};
  /// Air Filter Maintenance.
  bool dusFull{};
  /// ECO mode.
  bool eco{};
  /// Error Code. Known: 0x26 - Water Full.
  uint8_t errInfo{};
  /// Fan Speed.
  uint8_t fanSpeed{FAN_AUTO};
  // Silky Cool.
  bool hasNoWindFeel{};
  /// Set Target Humidity in Smart Dry Mode. Fan Speed must be AUTO.
  uint8_t humidity{};
  ///
  uint8_t light{};
  /// Mode.
  uint8_t mode{MODE_COOL};
  /// Device Status: On/Off.
  bool powerStatus{true};
  /// Electric Auxiliary Heater
  bool ptcAssis{};
  /// 39 Self Cleaning.
  bool selfClean{};
  /// Target Temperature. Always in Celsius.
  uint8_t setTemperature{26};
  /// Target Temperature +0.5.
  bool setTemperature_dot{};
  /// Sleep Preset.
  bool sleepFunc{};
  /// Display Temperature in Fahrenheits.
  bool tempUnit{};
  /// Current `On Timer` minutes.
  uint16_t timer_on{};
  /// Current `Off Timer` minutes.
  uint16_t timer_off{};
  /// Turbo Preset.
  bool turbo{};

  /// Swing Mode.
  SwingMode swing{};

  /// Indoor Temperature. Only in Status. Always in Celsius.
  float indoor_temp{};
  /// Outdoor Temperature. Only in Status. Always in Celsius.
  float outdoor_temp{};

  bool alarmSleep{};
  bool catchCold{};
  bool changeCosySleep{};
  bool childSleepMode{};
  bool cleanUp{};
  bool coolFan{};
  uint8_t cosySleep{};
  bool double_temp{};
  bool dryClean{};
  bool errMark{};
  bool exchangeAir{};
  bool feelOwn{};
  bool imodeResume{};
  bool lowFreqFan{};
  bool naturalFan{};
  bool nightLight{};
  bool peakElec{};
  bool powerSaver{};
  bool ptcButton{};
  uint8_t pwmMode{};
  bool save{};
  bool selfFeelOwn{};
  uint8_t setExpand{};
  bool setExpand_dot{};
  bool test2{};
  bool timerMode{};
  bool wiseEye{};
};

struct B1Status {
  uint8_t hWindDirection{};
  uint8_t vWindDirection{};
  uint8_t indoorHumidity{};
  std::vector<uint8_t> mMasterValues{};
  std::vector<uint8_t> mSlaveValues{};
  bool isSelfCleanOn{};
  uint8_t breezeType{1};
  bool isBuzzerOn{true};
  bool isNoWindFeelOn{false};
  bool isSoundOn{false};
  bool isBlowingPeopleOn{false};
  bool isAvoidPeopleOn{false};
  bool isOneKeyNoWindOnMeOn{false};
  bool isCombineBreezeOn{false};
  bool isSmartEyeOn{false};
  bool DehumLight{false};
};

class FrameStatusData : public FrameData {
 public:
  /**
   * @brief Checks for `FrameData` is convertible to `FrameStatusData`.
   * @param data reference to `FrameData` instance.
   * @return `true` if `FrameData` is `FrameStatusData`.
   */
  static bool hasStatusData(const FrameData &data);

  /**
   * @brief Make `0x40 Set status` command from `DeviceStatus`.
   *
   * @param s reference to `DeviceStatus`.
   */
  static FrameData writeStatus(const DeviceStatus &s);

  /**
   * @brief Updates `DeviceStatus`.
   *
   * @param s reference to `DeviceStatus`.
   */
  void readStatus(DeviceStatus &s) const;

 private:
  void m_command40(const DeviceStatus &s);
  void m_statusC0(DeviceStatus &s) const;
  void m_statusA0(DeviceStatus &s) const;
  void m_statusA1(DeviceStatus &s) const;
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
