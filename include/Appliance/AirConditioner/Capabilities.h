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
#include "Frame/PropertiesData.h"
#include "Appliance/CapabilitiesBase.h"

namespace dudanov {
namespace midea {
namespace ac {

/**
 * @brief Capability UUID.
 *
 */
enum CapabilityUUID : PropertyUUID {
  UUID_VWIND = 0x0009,          /**< `Vertical Air Flow Direction`. Values: 1, 25, 50, 75, 100. */
  UUID_HWIND = 0x000A,          /**< `Horizontal Air Flow Direction`. Values: 1, 25, 50, 75, 100. */
  UUID_HUMIDITY = 0x0015,       /**< `Indoor Humidity`. */
  UUID_SILKY_COOL = 0x0018,     /**< `Silky Cool`. */
  UUID_ECO_EYE = 0x0030,        /**< `ECO Intelligent Eye`. */
  UUID_WIND_ON_ME = 0x0032,     /**< `Wind ON me`. Only in `COOL` and `HEAT`. Turn on all swing. */
  UUID_WIND_OFF_ME = 0x0033,    /**< `Wind OFF me`. Only in `COOL` and `HEAT`. Turn off all swing. */
  UUID_CLEAN = 0x0039,          /**< `Active Clean`. */
  UUID_BREEZE_AWAY = 0x0042,    /**< `Breeze Away`. */
  UUID_BREEZELESS = 0x0043,     /**< `Breezeless`. Values: 1 (Off), 2 (Away), 3 (Mild), 4 (Less). */
  UUID_FAN = 0x0210,            /**< Supported Fan Speeds. */
  UUID_ECO = 0x0212,            /**< `ECO`. */
  UUID_8HEAT = 0x0213,          /**< `8°C Frost Protection`. */
  UUID_MODES = 0x0214,          /**< Supported Modes. */
  UUID_SWING = 0x0215,          /**< Swing Modes. */
  UUID_POWER = 0x0216,          /**< Power Features. */
  UUID_FILTER = 0x0217,         /**< Air Filter Features. (dusFull) */
  UUID_AUX_HEATER = 0x0219,     /**< Electric Auxiliary Heating. */
  UUID_TURBO = 0x021A,          /**< Turbo Mode Features. */
  UUID_DRY = 0x021F,            /**< Drying Modes. */
  UUID_FAHRENHEIT = 0x0222,     /**< Fahrenheit Support. */
  UUID_LIGHT = 0x0224,          /**< LED Control. Values: 0 (Off), 7 (On). */
  UUID_TEMP = 0x0225,           /**< Temperature Ranges. */
  UUID_BUZZER = 0x022C,         /**< Buzzer. Values: 0 (Off), 1 (On). */
  UUID_TWINS = 0x0232,          /**< Slave. Values in 0x0231 B1. */
  UUID_FOUR_DIRECTION = 0x0233, /**< Master. Values in 0x0230 B1. */
};

/**
 * @brief Capabilities data.
 *
 */
class Capabilities : public CapabilitiesBase {
 public:
  /**
   * @brief Dump capabilities report to logs.
   *
   */
  void dump() const override;

  /**
   * @brief Checks if any of 0xB1 capabilities is supported.
   *
   * @return `true` if any of 0xB1 capabilities is supported.
   */
  bool isNeedB1Query() const override;

  /* Capabilities */

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

  /// Fan Speeds coded data.
  uint8_t hasWindSpeed{0};

  /// `Wind On Me` supported.
  bool hasBlowingPeople{false};

  /// `Wind Off Me` supported.
  bool hasAvoidPeople{false};

  /// `Active Clean` supported.
  bool hasSelfClean{false};

  /// `Breeze Away` supported.
  bool hasOneKeyNoWindOnMe{false};

  /// `Breezeless` supported.
  bool hasBreeze{false};

  /// `ECO Intelligent Eye` supported.
  bool hasSmartEye{false};

  /// `Sound` supported.
  bool hasBuzzer{false};

  /// `Indoor Humidity` supported.
  bool hasIndoorHumidity{false};

  /// `Horizontal Wind Direction` supported.
  bool hasHorizontalWind{false};

  /// `Vertical Wind Direction` supported.
  bool hasVerticalWind{false};

  /// Multi Split.
  bool isTwins{false};

  /// Cassette Four Direction.
  bool isFourDirection{false};

  /// Fahrenheits display units supported.
  bool unitChangeable{true};

  /// `Cool Mode` supported.
  bool cool{true};

  // `Heat Mode` supported.
  bool hot{true};

  /// `Dry Mode` supported.
  bool dry{true};

  /// `Auto Mode` supported.
  bool auto_{true};

  /// `Fan Mode` supported.
  bool wind{true};

  /// `Cool Turbo Mode` supported.
  bool strongCool{true};

  /// `Vertical Swing` supported.
  bool leftrightFan{true};

  /// `Horizontal Swing` supported.
  bool updownFan{false};

  /// `ECO Preset` supported.
  bool eco{false};

  /// `Special ECO` supported.
  bool special_eco{false};

  /// `Frost Protection` supported.
  bool eightHot{false};

  /// `Power Usage Report` supported.
  bool powerCal{false};

  /// `Power Limit` supported.
  bool powerCalSetting{false};

  /// `Air Filter Cleaning Reminder` supported.
  bool nestCheck{false};

  /// `Air Filter Replacement Reminder` supported.
  bool nestNeedChange{false};

  /// `Electric Auxiliary Heater` supported.
  bool dianfure{false};

  /// `Heat Turbo` supported.
  bool strongHot{false};

  /// `Silky Cool` supported.
  bool hasNoWindFeel{false};

  /// `Dry Mode Auto` supported.
  bool hasAutoClearHumidity{false};

  /// `Dry Mode Smart` supported.
  bool hasHandClearHumidity{false};

  /// `Decimal Point` supported.
  bool isHavePoint{false};

  /// `Fan Speed` in percents.
  bool hasNoWindSpeed{false};

 protected:
  void m_setCapability(const PropertiesReader &s) override;
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
