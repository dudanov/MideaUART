/**
 * @file Capabilities.h
 * @author Sergey V. DUDANOV (sergey.dudanov@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07-02
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <Arduino.h>
#include <set>

namespace dudanov {
namespace midea {

class FrameData;

namespace ac {

/**
 * @brief Feature
 *
 *
 *
 *
 */
enum Feature : uint16_t {
  FRESH_AIR = 0x004B,       /**< Fresh Air */
  JET_COOL = 0x0067,        /**< Cool Flash */
  FAN_SPEED = 0x0210,       /**< Supported Fan Speeds */
  ECO_MODES = 0x0212,       /**< ECO */
  EIGHT_HEAT = 0x0213,      /**< 8°C Heat */
  MODES = 0x0214,           /**< Supported Modes */
  SWING_MODES = 0x0215,     /**< Swing Modes */
  POWER_FUNC = 0x0216,      /**< Power Features */
  AIR_FILTER = 0x0217,      /**< Air Filter Features. (dusFull) */
  AUX_HEATER = 0x0219,      /**< Electric Auxiliary Heating */
  TURBO_MODES = 0x021A,     /**< Turbo Mode Features */
  HUMIDITY = 0x021F,        /**< Drying Modes */
  UNIT_CHANGEABLE = 0x0222, /**< Fahrenheit Support */
  LED_LIGHT = 0x0224,       /**< LED Control. Values: 0 (Off), 7 (On). */
  TEMP_RANGES = 0x0225,     /**< Temperature Ranges */

  /* B1 Command Features. */

  V_WIND_DIRECTION = 0x0009,  /**< Vertical Air Flow Direction. Values: 1, 25, 50, 75, 100. */
  H_WIND_DIRECTION = 0x000A,  /**< Horizontal Air Flow Direction. Values: 1, 25, 50, 75, 100. */
  INDOOR_HUMIDITY = 0x0015,   /**< Indoor Humidity */
  SILKY_COOL = 0x0018,        /**< Silky Cool */
  ECO_EYE = 0x0030,           /**< ECO Intelligent Eye */
  WIND_ON_ME = 0x0032,        /**< Wind ON me. Only in `COOL` and `HEAT`. Turn on all swing. */
  WIND_OFF_ME = 0x0033,       /**< Wind OFF me. Only in `COOL` and `HEAT`. Turn off all swing. */
  ACTIVE_CLEAN = 0x0039,      /**< Active Clean */
  BREEZE_AWAY = 0x0042,       /**< Breeze Away */
  BREEZE = 0x0043,            /**< Breeze. Values: 1 (Off), 2 (Away), 3 (Mild), 4 (Less). */
  BUZZER = 0x022C,            /**< Buzzer. Values: 0 (Off), 1 (On). */
  IS_TWINS = 0x0232,          /**< Slave */
  IS_FOUR_DIRECTION = 0x0233, /**< Master */
};

/**
 * @brief Frame for request appliance capabilities.
 *
 */
class CmdB5 {
 public:
  // Read AC capabilities from frame data
  uint8_t read(const FrameData &data);
  // Dump capabilities
  void dump() const;
  // Checking for need sending B1 query
  bool isNeedB1Query() const;

  CmdB5 &setBaseFunc();
  CmdB5 &toOnlyCool();
  CmdB5 &toAllEnable();
  CmdB5 &toOnlyHot();
  CmdB5 &toSubCool();

  enum ACType : uint8_t {
    COLD,      // toOnlyCool()
    COLD_HOT,  // toAllEnable()
    HOT,       // toOnlyHot()
    COLD_SUB,  // toSubCool()
    COLD_SUB_COLD_HOT,
    COLD_SUB_COLD,
  };

  ACType getDeviceType() const { return static_cast<ACType>(this->hotcold); }

  /* B5 Features */

  /// Wind On Me supported.
  bool hasBlowingPeople{false};
  /// Wind Off Me supported.
  bool hasAvoidPeople{false};
  /// Active Clean supported.
  bool hasSelfClean{false};
  /// Breeze Away supported.
  bool hasOneKeyNoWindOnMe{false};
  /// Breezeless Preset supported.
  bool hasBreeze{false};
  /// ECO Intelligent Eye supported.
  bool hasSmartEye{false};
  /// Sound supported.
  bool hasBuzzer{false};
  /// Indoor Humidity supported.
  bool hasIndoorHumidity{false};
  /// Horizontal Wind Direction supported.
  bool hasHorizontalWind{false};
  /// Vertical Wind Direction supported.
  bool hasVerticalWind{false};

  bool isTwins{false};
  bool isFourDirection{false};

  /// Fahrenheits units supported.
  bool unitChangeable{true};
  /// Cool Mode supported.
  bool cool{true};
  // Heat Mode supported.
  bool hot{true};
  /// Dry Mode supported.
  bool dry{true};
  /// Auto Mode supported.
  bool auto1{true};
  /// FanOnly Mode supported.
  bool wind{true};
  bool mutilTemp{true};
  // bool selfcheck{true};
  /// Cool Turbo Mode supported.
  bool strongCool{true};
  /// Vertical Swing supported.
  bool leftrightFan{true};
  /// Horizontal Swing supported.
  bool updownFan{false};
  /// ECO Preset supported.
  bool eco{false};
  /// Special ECO Preset supported.
  bool special_eco{false};
  /// Frost Protection Preset supported.
  bool eightHot{false};
  bool powerCal{false};
  bool powerCalSetting{false};
  bool powerCalBCD{false};
  /// Air Filter Cleaning Reminder supported.
  bool nestCheck{false};
  /// Air Filter Replacement Reminder supported.
  bool nestNeedChange{false};
  /// Electric auxiliary heat supported.
  bool dianfure{false};
  /// Heat Turbo preset supported.
  bool strongHot{false};
  /// Silky Cool Preset supported.
  bool hasNoWindFeel{false};
  /// Dry Mode Auto supported.
  bool hasAutoClearHumidity{false};
  /// Dry Mode Smart supported.
  bool hasHandClearHumidity{false};
  /// Decimal Point supported.
  bool isHavePoint{false};
  /// Cool Flash Preset supported.
  bool hasJetCool{false};
  bool isJetCoolEnable{false};

  /// Fan Speed in percents.
  bool hasNoWindSpeed;
  /// Fan Speeds coded data.
  uint8_t hasWindSpeed;
  /// Base features.
  uint8_t hotcold;
  /// Supported Maximum Temperature in Cool Mode.
  uint8_t cool_adjust_up_temp{30};
  /// Supported Minimal Temperature in Cool Mode.
  uint8_t cool_adjust_down_temp{17};
  /// Supported Maximum Temperature in Auto Mode.
  uint8_t auto_adjust_up_temp{30};
  /// Supported Minimal Temperature in Auto Mode.
  uint8_t auto_adjust_down_temp{17};
  /// Supported Maximum Temperature in Heat Mode.
  uint8_t hot_adjust_up_temp{30};
  /// Supported Minimal Temperature in Heat Mode.
  uint8_t hot_adjust_down_temp{17};
  /// LED type
  uint8_t lightType{0};

  /// Fresh Air Preset supported.
  bool hasFreshAir{false};
  bool isFreshAirEnable{false};
  uint8_t fresh_air_switch{0};
  uint8_t fresh_air_speed{0};
  uint8_t fresh_air_temp{0};
  uint8_t fresh_air_mode{0};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
