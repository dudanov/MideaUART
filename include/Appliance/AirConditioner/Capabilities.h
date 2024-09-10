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

#include "Frame/PropertiesData.h"

namespace dudanov {
namespace midea {
namespace ac {

/**
 * @brief Feature
 *
 */
enum Feature : PropertyUUID {
  FEATURE_VWIND = 0x0009,          /**< Vertical Air Flow Direction. Values: 1, 25, 50, 75, 100. */
  FEATURE_HWIND = 0x000A,          /**< Horizontal Air Flow Direction. Values: 1, 25, 50, 75, 100. */
  FEATURE_HUMIDITY = 0x0015,       /**< Indoor Humidity. */
  FEATURE_SILKY_COOL = 0x0018,     /**< Silky Cool. */
  FEATURE_ECO_EYE = 0x0030,        /**< ECO Intelligent Eye. */
  FEATURE_WIND_ON_ME = 0x0032,     /**< Wind ON me. Only in `COOL` and `HEAT`. Turn on all swing. */
  FEATURE_WIND_OFF_ME = 0x0033,    /**< Wind OFF me. Only in `COOL` and `HEAT`. Turn off all swing. */
  FEATURE_CLEAN = 0x0039,          /**< Active Clean. */
  FEATURE_BREEZE_AWAY = 0x0042,    /**< Breeze Away. */
  FEATURE_BREEZELESS = 0x0043,     /**< Breezeless. Values: 1 (Off), 2 (Away), 3 (Mild), 4 (Less). */
  FEATURE_FAN = 0x0210,            /**< Supported Fan Speeds. */
  FEATURE_ECO = 0x0212,            /**< ECO. */
  FEATURE_8HEAT = 0x0213,          /**< 8°C Heat. */
  FEATURE_MODES = 0x0214,          /**< Supported Modes. */
  FEATURE_SWING = 0x0215,          /**< Swing Modes. */
  FEATURE_POWER = 0x0216,          /**< Power Features. */
  FEATURE_FILTER = 0x0217,         /**< Air Filter Features. (dusFull) */
  FEATURE_AUX_HEATER = 0x0219,     /**< Electric Auxiliary Heating. */
  FEATURE_TURBO = 0x021A,          /**< Turbo Mode Features. */
  FEATURE_DRY = 0x021F,            /**< Drying Modes. */
  FEATURE_FAHRENHEIT = 0x0222,     /**< Fahrenheit Support. */
  FEATURE_LIGHT = 0x0224,          /**< LED Control. Values: 0 (Off), 7 (On). */
  FEATURE_TEMP = 0x0225,           /**< Temperature Ranges. */
  FEATURE_BUZZER = 0x022C,         /**< Buzzer. Values: 0 (Off), 1 (On). */
  FEATURE_TWINS = 0x0232,          /**< Slave. Values in 0x0231 B1. */
  FEATURE_FOUR_DIRECTION = 0x0233, /**< Master. Values in 0x0230 B1. */
};

/**
 * @brief Capabilities data.
 *
 */
class Capabilities {
 public:
  /**
   * @brief Reads capabilities from 0xB5 frame data.
   *
   * @param data Reference to `PropertiesData` instance.
   * @return If zero - no more data is required. Else `ID` of next frame.
   */
  uint8_t read(const PropertiesData &data);

  /**
   * @brief Dump capabilities report to logs.
   *
   */
  void dump() const;

  /**
   * @brief Checks if any of 0xB1 capabilities is supported.
   *
   * @return `true` if any of 0xB1 capabilities is supported.
   */
  bool isNeedB1Query() const;

  /* Capabilities */

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

  /// Multi Split.
  bool isTwins{false};

  /// Cassette Four Direction.
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
  bool auto_{true};

  /// FanOnly Mode supported.
  bool wind{true};

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

  /// Power Usage Report supported.
  bool powerCal{false};

  /// Power Limit supported.
  bool powerCalSetting{false};

  /// Air Filter Cleaning Reminder supported.
  bool nestCheck{false};

  /// Air Filter Replacement Reminder supported.
  bool nestNeedChange{false};

  /// Electric Auxiliary Heat supported.
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

  /// Fan Speed in percents.
  bool hasNoWindSpeed{false};

  /// Fan Speeds coded data.
  uint8_t hasWindSpeed{0};

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

  /// LED Light. Values: 0 (Off), 7 (On).
  uint8_t lightType{0};

 protected:
  void m_setFuncEnable(const PropertiesData::PropertiesReader &data);
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
