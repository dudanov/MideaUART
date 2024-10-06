#pragma once
#include <deque>
#include "Frame/FrameIO.h"
#include "Helpers/Timer.h"
#include "Helpers/Logger.h"

namespace dudanov {
namespace midea {

enum AutoconfStatus : uint8_t {
  AUTOCONF_DISABLED,
  AUTOCONF_PROGRESS,
  AUTOCONF_DONE,
  AUTOCONF_ERROR,
};

enum ResponseStatus : uint8_t {
  RESPONSE_OK,
  RESPONSE_PARTIAL,
  RESPONSE_WRONG,
};

using Handler = std::function<void()>;
using ResponseHandler = std::function<ResponseStatus(FrameData)>;
using OnStateCallback = std::function<void()>;

class ApplianceBase {
 public:
  /// Setup
  void setup();
  /// Loop
  void loop();

  /* ############################## */
  /* ### COMMUNICATION SETTINGS ### */
  /* ############################## */

  /// Set serial stream.
  void setStream(IOStream *io) { m_frameio.setStream(io); }

  /// Set minimal period between requests.
  void setPeriod(uint32_t period) { m_period = period; }

  uint32_t getPeriod() const { return m_period; }

  /// Set waiting response timeout.
  void setTimeout(uint32_t timeout) { m_timeout = timeout; }

  uint32_t getTimeout() const { return m_timeout; }

  /// Set number of request attempts.
  void setNumAttempts(uint8_t numAttempts) { m_numAttempts = numAttempts; }

  uint8_t getNumAttempts() const { return m_numAttempts; }

  /// Set beeper feedback.
  void setBeeper(bool value);

  /// Add listener for appliance state.
  void addOnStateCallback(OnStateCallback cb) { m_stateCallbacks.push_back(cb); }

  void sendUpdate() {
    for (auto &cb : m_stateCallbacks)
      cb();
  }

  AutoconfStatus getAutoconfStatus() const { return m_autoconfStatus; }

  void setAutoconf(bool state) { m_autoconfStatus = state ? AUTOCONF_PROGRESS : AUTOCONF_DISABLED; }

  static void setLogger(LoggerFn logger) { dudanov::setLogger(logger); }

 protected:
  /// Callbacks of state subscribers.
  std::vector<OnStateCallback> m_stateCallbacks;

  /// Timer manager.
  TimerManager m_timerManager{};

  /// Status of autoconfig process.
  AutoconfStatus m_autoconfStatus{};

  // Beeper feedback flag.
  bool m_beeper{};

  void m_queueNotify(FrameType type, FrameData data) { m_queueRequest(type, std::move(data), nullptr); }

  void m_queueRequest(FrameType type, FrameData &&data, ResponseHandler onData, Handler onSucess = nullptr,
                      Handler onError = nullptr);

  void m_queueRequestPriority(FrameType type, FrameData &&data, ResponseHandler onData = nullptr,
                              Handler onSucess = nullptr, Handler onError = nullptr);

  void m_sendFrame(FrameType type, const FrameData &data);

  /// Setup for appliances.
  virtual void m_setup() {}

  /// Loop for appliances.
  virtual void m_loop() {}

  /// Calling then ready for request.
  virtual void m_onIdle() {}

  /// Calling on receiving request.
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

  void m_sendNetworkNotify(FrameType typeID = NETWORK_NOTIFY);

  void m_handler(const Frame &s);

  bool m_isWaitForResponse() const { return m_request != nullptr; }

  void m_resetAttempts() { m_remainAttempts = m_numAttempts; }

  void m_destroyRequest();

  void m_resetTimeout();

  void m_sendRequest(Request *req) { m_sendFrame(req->requestType, req->request); }

  /// Frame receiver with dynamic buffer.
  FrameIO m_frameio{};

  /// Network status timer.
  Timer m_networkTimer{};

  /// Waiting response timer.
  Timer m_responseTimer{};

  /// Request period timer.
  Timer m_periodTimer{};

  /// Requests queue.
  std::deque<Request *> m_queue;

  /// Current request.
  Request *m_request{nullptr};

  // Remaining request attempts
  uint8_t m_remainAttempts{};

  /// Period flag
  bool m_isBusy{};

  /* ############################## */
  /* ### COMMUNICATION SETTINGS ### */
  /* ############################## */

  /// Minimal period between requests
  uint32_t m_period{1000};

  /// Waiting response timeout
  uint32_t m_timeout{2000};

  /// Number of request attempts
  uint8_t m_numAttempts{3};
};

}  // namespace midea
}  // namespace dudanov
