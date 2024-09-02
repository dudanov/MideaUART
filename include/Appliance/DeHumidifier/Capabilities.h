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

namespace dh {

/**
 * @brief Feature
 *
 *
 *
 *
 */
enum Feature : uint16_t {
  FAN_SPEED = 0x0210,      /**< Motor Type */
  MODES = 0x0214,          /**< Buzzer. Values: 0 (Off), 1 (On). */
  VERTICAL_SWING = 0x0215, /**< Buzzer. Values: 0 (Off), 1 (On). */
  AIR_FILTER = 0x0217,     /**< Filter Screen */
  WATER_PUMP = 0x021D,     /**< Water Pump */
  FILTER_SCREEN = 0x021E,  /**< Filter Screen */
  SMART = 0x021F,          /**< Filter Screen */
  DRY_CLOTHES = 0x0220,    /**< Filter Screen */
  LIGHT = 0x0224,          /**< LED Control. Values: 0 (Off), 7 (On). */
  WATER_FULL = 0x022D,     /**< Water Full */
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
  bool auto_{true};
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

}  // namespace dh
}  // namespace midea
}  // namespace dudanov
