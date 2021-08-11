#include "Appliance/ApplianceBase.h"
#include <ESP8266WiFi.h>

namespace dudanov {
namespace midea {

ResponseStatus ApplianceBase::Request::callHandler(const Frame &frame) {
  if (!frame.hasType(this->requestType))
    return ResponseStatus::RESPONSE_WRONG;
  if (this->onData == nullptr)
    return RESPONSE_OK;
  return this->onData(frame.getData());
}

bool ApplianceBase::FrameReceiver::read(Stream *stream) {
  while (stream->available() > 0) {
    const uint8_t data = stream->read();
    const uint8_t len = this->size();
    if (len == OFFSET_START && data != START_BYTE)
      continue;
    if (len == OFFSET_LENGTH && data <= OFFSET_DATA) {
      this->clear();
      continue;
    }
    this->m_data.push_back(data);
    if (len >= OFFSET_LENGTH && len >= this->m_len())
      return true;
  }
  return false;
}

void ApplianceBase::setup() {
  this->m_timerManager.registerTimer(this->m_periodTimer);
  this->m_timerManager.registerTimer(this->m_networkTimer);
  this->m_timerManager.registerTimer(this->m_responseTimer);
  this->m_networkTimer.setCallback([this](Timer *timer) {
    this->m_sendNetworkNotify();
    timer->reset();
  });
  this->m_networkTimer.start(2 * 60 * 1000);
  this->m_networkTimer.call();
  this->m_setup();
}

void ApplianceBase::loop() {
  // Timers task
  m_timerManager.task();
  // Loop for appliances
  m_loop();
  // Frame receiving
  while (this->m_receiver.read(this->m_stream)) {
    //this->logger_->log(LOG_LEVEL_DEBUG, hexdump (this->receiver_.data(), this->receiver_.size());
    this->m_protocol = this->m_receiver.getProtocol();
    this->m_handler(this->m_receiver);
    this->m_receiver.clear();
  }
  if (this->m_isBusy || this->m_isWaitForResponse())
    return;
  if (this->m_queue.empty()) {
    this->m_onIdle();
    return;
  }
  this->m_request = this->m_queue.front();
  this->m_queue.pop_front();
  //ESP_LOGD(TAG, "Getting and sending a request from the queue...");
  this->m_sendRequest(this->m_request);
  this->m_resetAttempts();
  this->m_resetTimeout();
}

void ApplianceBase::m_handler(const Frame &frame) {
  if (this->m_isWaitForResponse()) {
    auto result = this->m_request->callHandler(frame);
    if (result != RESPONSE_WRONG) {
      if (result == RESPONSE_OK) {
        this->m_destroyRequest();
      } else {
        this->m_resetAttempts();
        this->m_resetTimeout();
      }
      return;
    }
  }
  /* HANDLE REQUESTS */
  if (frame.hasType(QUERY_NETWORK)) {
    this->m_sendNetworkNotify(QUERY_NETWORK);
    return;
  }
  this->m_onRequest(frame);
}

static uint8_t getSignalStrength() {
  const int32_t dbm = WiFi.RSSI();
  if (dbm > -63)
    return 4;
  if (dbm > -75)
    return 3;
  if (dbm > -88)
    return 2;
  return 1;
}

void ApplianceBase::m_sendNetworkNotify(FrameType msgType) {
  NetworkNotifyData notify{};
  notify.setConnected(WiFi.isConnected());
  notify.setSignalStrength(getSignalStrength());
  notify.setIP(WiFi.localIP());
  if (msgType == NETWORK_NOTIFY) {
    //ESP_LOGD(TAG, "Enqueuing a DEVICE_NETWORK(0x0D) notification...");
    this->m_queueRequest(msgType, notify);
  } else {
    //ESP_LOGD(TAG, "Answer to QUERY_NETWORK(0x63) request...");
    this->m_sendFrame(msgType, notify);
  }
}

void ApplianceBase::m_resetTimeout() {
  this->m_responseTimer.setCallback([this](Timer *timer) {
    //ESP_LOGD(TAG, "Response timeout...");
    if (!--this->m_remain_attempts) {
      if (this->m_request->onError != nullptr)
        this->m_request->onError();
      this->m_destroyRequest();
      return;
    }
    //ESP_LOGD(TAG, "Sending request again. Attempts left: %d...", this->remain_attempts_);
    this->m_sendRequest(this->m_request);
    this->m_resetTimeout();
  });
  this->m_responseTimer.start(this->m_timeout);
}

void ApplianceBase::m_destroyRequest() {
  //ESP_LOGD(TAG, "Destroying the request...");
  this->m_responseTimer.stop();
  delete this->m_request;
  this->m_request = nullptr;
}

void ApplianceBase::m_sendFrame(FrameType type, const FrameData &data) {
  Frame frame(this->m_appType, this->m_protocol, type, data);
  //ESP_LOGD(TAG, "TX: %s", frame.to_string().c_str());
  this->m_stream->write(frame.data(), frame.size());
  this->m_isBusy = true;
  this->m_periodTimer.setCallback([this](Timer *timer) {
    this->m_isBusy = false;
    timer->stop();
  });
  this->m_periodTimer.start(this->m_period);
}

void ApplianceBase::m_queueRequest(FrameType type, const FrameData &data, ResponseHandler onData, Handler onSucess, Handler onError) {
  //ESP_LOGD(TAG, "Enqueuing the request...");
  this->m_queue.push_back(new Request{data, std::move(onData), std::move(onSucess), std::move(onError), type});
}

void ApplianceBase::m_queueRequestPriority(FrameType type, const FrameData &data, ResponseHandler onData, Handler onSucess, Handler onError) {
  //ESP_LOGD(TAG, "Priority request queuing...");
  this->m_queue.push_front(new Request{data, std::move(onData), std::move(onSucess), std::move(onError), type});
}

} // namespace midea
} // namespace dudanov
