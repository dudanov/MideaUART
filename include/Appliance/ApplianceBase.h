#pragma once
#include <Arduino.h>
#include "Frame/Frame.h"
#include "Frame/FrameData.h"
#include "Helpers/Timer.h"
#include <deque>

namespace dudanov {
namespace midea {

/// Enum for all modes a Midea device can be in.
enum Mode : uint8_t {
  ///
  MODE_OFF,
  /// The Midea device is set to automatically change the heating/cooling cycle
  MODE_AUTO,
  /// The Midea device is manually set to cool mode (not in auto mode!)
  MODE_COOL,
  /// The Midea device is manually set to dry mode
  MODE_DRY,
  /// The Midea device is manually set to heat mode (not in auto mode!)
  MODE_HEAT,
  /// The Midea device is manually set to fan only mode
  MODE_FAN_ONLY,
};

/// Enum for all modes a Midea fan can be in
enum FanMode : uint8_t {
  /// The fan mode is set to Auto
  FAN_AUTO = 102,
  /// The fan mode is set to Silent
  FAN_SILENT = 20,
  /// The fan mode is set to Low
  FAN_LOW = 40,
  /// The fan mode is set to Medium
  FAN_MEDIUM = 60,
  /// The fan mode is set to High
  FAN_HIGH = 80,
  /// The fan mode is set to Turbo
  FAN_TURBO = 100,
};

/// Enum for all modes a Midea swing can be in
enum SwingMode : uint8_t {
  /// The sing mode is set to Off
  SWING_OFF = 0b0000,
  /// The fan mode is set to Both
  SWING_BOTH = 0b1111,
  /// The fan mode is set to Vertical
  SWING_VERTICAL = 0b1100,
  /// The fan mode is set to Horizontal
  SWING_HORIZONTAL = 0b0011,
};

/// Enum for all presets a Midea can be in
enum Preset : uint8_t {
  /// None preset
  PRESET_NONE,
  /// The ECO preset
  PRESET_ECO,
  /// The TURBO preset
  PRESET_TURBO,
  /// The SLEEP preset
  PRESET_SLEEP,
  /// The FREEZE_PROTECTION preset
  PRESET_FREEZE_PROTECTION,
};

class Frame;
class ApplianceBase;

enum ApplianceType : uint8_t {
  DEHUMIDIFIER = 0xA1,
  AIR_CONDITIONER = 0xAC,
  AIR2WATER = 0xC3,
  FAN = 0xFA,
  CLEANER = 0xFC,
  HUMIDIFIER = 0xFD,
  BROADCAST = 0xFF
};

enum AutoconfStatus : uint8_t {
  AUTOCONF_DISABLED,
  AUTOCONF_PROGRESS,
  AUTOCONF_OK,
  AUTOCONF_ERROR,
};

enum ResponseStatus : uint8_t {
  RESPONSE_OK,
  RESPONSE_PARTIAL,
  RESPONSE_WRONG,
};

enum FrameType : uint8_t {
  DEVICE_CONTROL = 0x02,
  DEVICE_QUERY = 0x03,
  GET_ELECTRONIC_ID = 0x07,
  NETWORK_NOTIFY = 0x0D,
  QUERY_NETWORK = 0x63,
};

using Handler = std::function<void()>;
using ResponseHandler = std::function<ResponseStatus(FrameData)>;
using LoggerFn = std::function<void(LogLevel, const char *)>;
using IRFunction = std::function<void(const uint8_t *)>;
using OnStateCallback = std::function<void()>;

class ApplianceBase {
 public:
  ApplianceBase(ApplianceType type) : m_appType(type) {}
  /// Setup
  void setup();
  /// Loop
  void loop();

  static void setLogger(LoggerFn logger) { s_logger = logger; }
  static void setIRTransmitFunction(IRFunction func) { ApplianceBase::s_irTransmit = func; }
  static void log(LogLevel l, const char *s) {
    if (s_logger != nullptr)
      s_logger(l, s);
  }
  static void transmitIR(const uint8_t *data) {
    if (ApplianceBase::s_irTransmit != nullptr)
      ApplianceBase::s_irTransmit(data);
  }

  /* ############################## */
  /* ### COMMUNICATION SETTINGS ### */
  /* ############################## */

  /// Set serial stream
  void setStream(Stream *stream) { this->m_stream = stream; }
  /// Set minimal period between requests
  void setPeriod(uint32_t period) { this->m_period = period; }
  /// Set waiting response timeout
  void setTimeout(uint32_t timeout) { this->m_timeout = timeout; }
  /// Set number of request attempts
  void setNumAttempts(uint8_t numAttempts) { this->m_numAttempts = numAttempts; }
  /// Set beeper feedback
  void setBeeper(bool value) { this->m_beeper = value; }
  /// Add listener for appliance state
  void addOnStateCallback(OnStateCallback cb) { this->m_stateCallbacks.push_back(cb); }
  void sendUpdate() {
    for (auto &cb : this->m_stateCallbacks)
      cb();
  }
  AutoconfStatus getAutoconfStatus() const { return this->m_autoconfStatus; }
  void setAutoconf(bool state) { this->m_autoconfStatus = state ? AUTOCONF_PROGRESS : AUTOCONF_DISABLED; }

 protected:
  std::vector<OnStateCallback> m_stateCallbacks;
  // Timer manager
  TimerManager m_timerManager{};
  AutoconfStatus m_autoconfStatus{};
  static IRFunction s_irTransmit;
  static LoggerFn s_logger;
  // Beeper feedback flag
  bool m_beeper{};

  void m_queueRequest(FrameType type, const FrameData &data, ResponseHandler onData = nullptr, Handler onSucess = nullptr, Handler onError = nullptr);
  void m_queueRequestPriority(FrameType type, const FrameData &data, ResponseHandler onData = nullptr, Handler onSucess = nullptr, Handler onError = nullptr);
  void m_sendFrame(FrameType type, const FrameData &data);
  // Setup for appliances
  virtual void m_setup() {}
  // Loop for appliances
  virtual void m_loop() {}
  /// Calling then ready for request
  virtual void m_onIdle() {}
  /// Calling on receiving request
  virtual void m_onRequest(const Frame &frame) {}
 private:
  struct Request {
    FrameData request;
    ResponseHandler onData;
    Handler onSuccess;
    Handler onError;
    FrameType requestType;
    ResponseStatus callHandler(const Frame &data);
  };
  class FrameReceiver : public Frame {
  public:
    bool read(Stream *stream);
    void clear() { this->m_data.clear(); }
  };
  void m_sendNetworkNotify(FrameType msg_type = NETWORK_NOTIFY);
  void m_handler(const Frame &frame);
  bool m_isWaitForResponse() const { return this->m_request != nullptr; }
  void m_resetAttempts() { this->m_remain_attempts = this->m_numAttempts; }
  void m_destroyRequest();
  void m_resetTimeout();
  void m_sendRequest(Request *request) { this->m_sendFrame(request->requestType, request->request); }
  // Frame receiver with dynamic buffer
  FrameReceiver m_receiver{};
  // Network status timer
  Timer m_networkTimer{};
  // Waiting response timer
  Timer m_responseTimer{};
  // Request period timer
  Timer m_periodTimer{};
  // Queue requests
  std::deque<Request *> m_queue;
  // Current request
  Request *m_request{nullptr};
  // Remaining request attempts
  uint8_t m_remain_attempts{};
  // Appliance type
  ApplianceType m_appType;
  // Appliance protocol
  uint8_t m_protocol{};
  // Period flag
  bool m_isBusy{};

  /* ############################## */
  /* ### COMMUNICATION SETTINGS ### */
  /* ############################## */
  
  // Stream serial interface
  Stream *m_stream;
  // Minimal period between requests
  uint32_t m_period{1000};
  // Waiting response timeout
  uint32_t m_timeout{2000};
  // Number of request attempts
  uint8_t m_numAttempts{3};
};

}  // namespace midea_dongle
}  // namespace esphome
