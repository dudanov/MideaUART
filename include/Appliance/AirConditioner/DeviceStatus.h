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
  /// Indoor Temperature. Only in Status. Always in Celsius.
  float indoor_temp;
  /// Outdoor Temperature. Only in Status. Always in Celsius.
  float outdoor_temp;

  /// Current `On Timer` minutes.
  uint16_t timer_on;
  /// Current `Off Timer` minutes.
  uint16_t timer_off;

  /// Error Code. Known: 0x26 - Water Full.
  uint8_t errInfo;

  /// Mode.
  uint8_t mode : 3;
  /// Target Temperature. Always in Celsius.
  uint8_t setTemperature : 5;

  /// Fan Speed.
  uint8_t fanSpeed : 7;
  /// Beeper Feedback. Only in Command.
  bool beeper : 1;

  /// Humidity setpoint in Smart Dry Mode. Fan Speed must be AUTO.
  uint8_t humidity : 7;
  /// Frost Protection mode (8 Degrees Heating).
  bool Eight_Hot : 1;

  /// Swing Mode.
  SwingMode swing : 4;
  /// LED Light.
  uint8_t light : 3;
  /// ECO mode.
  bool eco : 1;

  /// Device Status: On/Off.
  bool powerStatus : 1;
  uint8_t cosySleep : 2;
  uint8_t setExpand : 5;

  /// Wind Off Me. Only in `COOL` and `HEAT`. Turn OFF all Swing.
  // bool avoidPeople : 1;
  /// Wind On Me. Only in `COOL` and `HEAT`. Turn ON all Swing.
  // bool blowingPeople : 1;
  /// Breeze Away.
  // bool noWindOnMe : 1;
  /// 39 Self Cleaning.
  // bool selfClean : 1;

  uint8_t pwmMode : 4;

  /// Air Filter Maintenance.
  bool dusFull : 1;
  /// Electric Auxiliary Heater
  bool ptcAssis : 1;
  /// Target Temperature +0.5.
  bool setTemperature_dot : 1;
  /// Sleep Preset.
  bool sleepFunc : 1;
  /// Display Temperature in Fahrenheits.
  bool tempUnit : 1;
  /// Turbo Preset.
  bool turbo : 1;
  bool catchCold : 1;
  bool childSleepMode : 1;
  bool cleanUp : 1;
  bool double_temp : 1;
  bool dryClean : 1;
  bool exchangeAir : 1;
  bool feelOwn : 1;
  bool imodeResume : 1;
  bool lowFreqFan : 1;
  bool naturalFan : 1;
  bool nightLight : 1;
  bool peakElec : 1;
  bool save : 1;
  bool setExpand_dot : 1;
  bool test2 : 1;
  bool timerMode : 1;

  /* COMMAND ONLY */

  /// Reset Air Filter Maintenance Timer. Only in Command.
  bool cleanFanTime : 1;
  bool alarmSleep : 1;
  bool changeCosySleep : 1;
  bool powerSaver : 1;
  bool ptcButton : 1;
  bool wiseEye : 1;

  /* STATUS ONLY */

  /// Silky Cool.
  bool hasNoWindFeel : 1;
  bool coolFan : 1;
  bool errMark : 1;
  bool selfFeelOwn : 1;
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
  size_t dsasd() { return sizeof(DeviceStatus); }
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
