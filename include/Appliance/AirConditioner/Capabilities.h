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
  FRESH_AIR = 0x004B,         /**< Fresh Air */
  JET_COOL = 0x0067,          /**< Cool Flash */
  FAN_SPEED = 0x0210,         /**< Supported Fan Speeds */
  ECO_MODES = 0x0212,         /**< ECO */
  EIGHT_HEAT = 0x0213,        /**< 8°C Heat */
  MODES = 0x0214,             /**< Supported Modes */
  SWING_MODES = 0x0215,       /**< Swing Modes */
  POWER_FUNC = 0x0216,        /**< Power Features */
  AIR_FILTER = 0x0217,        /**< Air Filter Features. (dusFull) */
  AUX_HEATER = 0x0219,        /**< Electric Auxiliary Heating */
  TURBO_MODES = 0x021A,       /**< Turbo Mode Features */
  HUMIDITY = 0x021F,          /**< Drying Modes */
  UNIT_CHANGEABLE = 0x0222,   /**< Fahrenheit Support */
  LED_LIGHT = 0x0224,         /**< LED Control. Values: 0 (Off), 7 (On). */
  TEMP_RANGES = 0x0225,       /**< Temperature Ranges */
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

  bool hasBlowingPeople{false};
  bool hasAvoidPeople{false};
  // Active clean
  bool hasSelfClean{false};
  bool hasOneKeyNoWindOnMe{false};
  bool hasBreeze{false};
  bool hasSmartEye{false};
  // Sound
  bool hasBuzzer{false};
  bool hasIndoorHumidity{false};
  // adjusting the direction of horizontal air flow
  bool hasHorizontalWind{false};
  // adjusting the direction of vertical air flow
  bool hasVerticalWind{false};
  bool isTwins{false};
  bool isFourDirection{false};

  // Fahrenheits
  bool unitChangeable{true};
  // Cool mode
  bool cool{true};
  // Heat mode
  bool hot{true};
  // Dry mode
  bool dry{true};
  // Auto mode
  bool auto1{true};
  // FanOnly mode
  bool wind{true};
  bool mutilTemp{true};
  bool selfcheck{true};
  // Cool Turbo preset
  bool strongCool{true};
  // Vertical swing
  bool leftrightFan{true};
  // Horizontal swing
  bool updownFan{false};
  // ECO preset
  bool eco{false};
  // Special ECO preset
  bool special_eco{false};
  // Frost Protection
  bool eightHot{false};
  bool powerCal{false};
  bool powerCalSetting{false};
  bool powerCalBCD{false};
  // Air Filter
  bool nestCheck{false};
  bool nestNeedChange{false};
  // Electric auxiliary heat
  bool dianfure{false};
  // Heat Turbo preset
  bool strongHot{false};
  bool hasNoWindFeel{false};
  bool hasAutoClearHumidity{false};
  bool hasHandClearHumidity{false};
  bool isHavePoint{false};
  bool hasJetCool{false};
  bool hasFreshAir{false};
  bool isJetCoolEnable{false};
  bool isFreshAirEnable{false};
  bool hasNoWindSpeed;
  uint8_t hasWindSpeed;
  uint8_t hotcold;
  // Maximum temperature in COOL mode
  uint8_t cool_adjust_up_temp{30};
  // Minimal temperature in COOL mode
  uint8_t cool_adjust_down_temp{17};
  // Maximum temperature in AUTO mode
  uint8_t auto_adjust_up_temp{30};
  // Minimal temperature in AUTO mode
  uint8_t auto_adjust_down_temp{17};
  // Maximum temperature in HEAT mode
  uint8_t hot_adjust_up_temp{30};
  // Minimal temperature in HEAT mode
  uint8_t hot_adjust_down_temp{17};
  uint8_t lightType{0};
  uint8_t fresh_air_switch{0};
  uint8_t fresh_air_speed{0};
  uint8_t fresh_air_temp{0};
  uint8_t fresh_air_mode{0};
};

}  // namespace ac
}  // namespace midea
}  // namespace dudanov
