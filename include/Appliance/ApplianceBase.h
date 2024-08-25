#pragma once
#include <deque>
#include <Arduino.h>
#include "Frame/Frame.h"
#include "Frame/FrameData.h"
#include "Helpers/Timer.h"
#include "Helpers/Logger.h"

namespace dudanov {
namespace midea {

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
using OnStateCallback = std::function<void()>;

class ApplianceBase {
 public:
  ApplianceBase(ApplianceType type) : m_appType(type) {}
  /// Setup
  void setup();
  /// Loop
  void loop();

  /* ############################## */
  /* ### COMMUNICATION SETTINGS ### */
  /* ############################## */

  /// Set serial stream
  void setStream(Stream *stream) { this->m_stream = stream; }
  /// Set minimal period between requests
  void setPeriod(uint32_t period) { this->m_period = period; }
  uint32_t getPeriod() const { return this->m_period; }
  /// Set waiting response timeout
  void setTimeout(uint32_t timeout) { this->m_timeout = timeout; }
  uint32_t getTimeout() const { return this->m_timeout; }
  /// Set number of request attempts
  void setNumAttempts(uint8_t numAttempts) { this->m_numAttempts = numAttempts; }
  uint8_t getNumAttempts() const { return this->m_numAttempts; }
  /// Set beeper feedback
  void setBeeper(bool value);
  /// Add listener for appliance state
  void addOnStateCallback(OnStateCallback cb) { this->m_stateCallbacks.push_back(cb); }
  void sendUpdate() {
    for (auto &cb : this->m_stateCallbacks)
      cb();
  }
  AutoconfStatus getAutoconfStatus() const { return this->m_autoconfStatus; }
  void setAutoconf(bool state) { this->m_autoconfStatus = state ? AUTOCONF_PROGRESS : AUTOCONF_DISABLED; }
  static void setLogger(LoggerFn logger) { dudanov::setLogger(logger); }

 protected:
  std::vector<OnStateCallback> m_stateCallbacks;
  // Timer manager
  TimerManager m_timerManager{};
  AutoconfStatus m_autoconfStatus{};
  // Beeper feedback flag
  bool m_beeper{};

  void m_queueNotify(FrameType type, FrameData data) { this->m_queueRequest(type, std::move(data), nullptr); }
  void m_queueRequest(FrameType type, FrameData data, ResponseHandler onData, Handler onSucess = nullptr, Handler onError = nullptr);
  void m_queueRequestPriority(FrameType type, FrameData data, ResponseHandler onData = nullptr, Handler onSucess = nullptr, Handler onError = nullptr);
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
  void m_sendNetworkNotify(FrameType msg_type = NETWORK_NOTIFY);
  void m_handler(const Frame &frame);
  bool m_isWaitForResponse() const { return this->m_request != nullptr; }
  void m_resetAttempts() { this->m_remainAttempts = this->m_numAttempts; }
  void m_destroyRequest();
  void m_resetTimeout();
  void m_sendRequest(Request *request) { this->m_sendFrame(request->requestType, request->request); }
  // Frame receiver with dynamic buffer
  Frame m_receiver{};
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
  uint8_t m_remainAttempts{};
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

}  // namespace midea
}  // namespace dudanov
