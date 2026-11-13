#include "Appliance/ApplianceBase.h"
#include "Helpers/Log.h"
#include <cstring>

#ifdef ARDUINO
  #ifdef ARDUINO_ARCH_ESP32
    #include <WiFi.h>
  #else
    #include <ESP8266WiFi.h>
  #endif
#else
  #if HAS_WIFI
    #include "esp_wifi.h"
    #include "esp_netif.h"
  #endif
#endif

namespace dudanov {
namespace midea {

static const char *TAG = "ApplianceBase";

ResponseStatus ApplianceBase::Request::callHandler(const Frame &frame) {
  if (!frame.hasType(this->requestType))
    return ResponseStatus::RESPONSE_WRONG;
  if (this->onData == nullptr)
    return RESPONSE_OK;
  return this->onData(frame.getData());
}

bool ApplianceBase::FrameReceiver::read(
    Stream *stream, uint8_t appliance) {
  // A previous extraction may have left a complete buffered frame.
  if (this->extractFrame(appliance))
    return true;

  while (stream->available()) {
    const int value = stream->read();

    // An unreadable byte does not establish that the UART is idle.
    if (value < 0)
      return false;

    // Extraction consumes or rejects complete candidates before
    // another byte is appended, leaving at most 255 pending bytes.
    this->m_input[this->m_inputSize++] =
        static_cast<uint8_t>(value);
    this->m_lastByteAt = static_cast<uint32_t>(millis());

    if (this->extractFrame(appliance))
      return true;
  }

  // Drain queued continuation bytes before considering expiry.
  const uint32_t idle_ms =
      static_cast<uint32_t>(millis()) - this->m_lastByteAt;

  return this->extractFrame(
      appliance, idle_ms >= PARTIAL_IDLE_MS);
}

void ApplianceBase::FrameReceiver::consume(size_t count) {
  this->m_inputSize -= count;

  if (this->m_inputSize) {
    std::memmove(
        this->m_input,
        this->m_input + count,
        this->m_inputSize);
  }
}

bool ApplianceBase::FrameReceiver::extractFrame(
    uint8_t appliance, bool expire_partial) {
  while (this->m_inputSize) {
    size_t prefix = 0;
    while (prefix < this->m_inputSize &&
           this->m_input[prefix] != START_BYTE) {
      ++prefix;
    }
    this->consume(prefix);

    if (!this->m_inputSize)
      return false;

    if (this->m_inputSize >= 2 &&
        this->m_input[OFFSET_LENGTH] <= OFFSET_DATA) {
      this->consume(1);
      continue;
    }

    // Reject false alignment before trusting the declared length.
    if (this->m_inputSize > OFFSET_APPTYPE &&
        this->m_input[OFFSET_APPTYPE] != appliance) {
      this->consume(1);
      continue;
    }

    const size_t total =
        this->m_inputSize >= 2
            ? size_t(this->m_input[OFFSET_LENGTH]) + 1
            : 2;

    if (this->m_inputSize < total) {
      if (!expire_partial)
        return false;

      // Abandon only this candidate's leading byte.
      this->consume(1);
      continue;
    }

    uint8_t checksum = 0;
    for (size_t i = OFFSET_LENGTH; i < total; ++i)
      checksum += this->m_input[i];

    if (checksum != 0) {
      // Preserve possible frame starts inside the failed candidate.
      this->consume(1);
      continue;
    }

    // Consume a valid frame whole, including embedded AA bytes.
    this->m_data.assign(
        this->m_input, this->m_input + total);
    this->consume(total);
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
  while (this->m_receiver.read(this->m_stream, this->m_appType)) {
    this->m_protocol = this->m_receiver.getProtocol();
    LOG_D(TAG, "RX: %s", this->m_receiver.toString().c_str());
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
  LOG_D(TAG, "Getting and sending a request from the queue...");
  this->m_sendRequest(this->m_request);
  if (this->m_request->onData != nullptr) {
    this->m_resetAttempts();
    this->m_resetTimeout();
  } else {
    this->m_destroyRequest();
  }
}

void ApplianceBase::m_handler(const Frame &frame) {
  if (this->m_isWaitForResponse()) {
    auto result = this->m_request->callHandler(frame);
    if (result != RESPONSE_WRONG) {
      if (result == RESPONSE_OK) {
        if (this->m_request->onSuccess != nullptr)
          this->m_request->onSuccess();
        this->m_destroyRequest();
      } else {
        this->m_resetAttempts();
        this->m_resetTimeout();
      }
      return;
    }
  }
  // ignoring responses on network notifies
  if (frame.hasType(NETWORK_NOTIFY))
    return;
  /* HANDLE REQUESTS */
  if (frame.hasType(QUERY_NETWORK)) {
    this->m_sendNetworkNotify(QUERY_NETWORK);
    return;
  }
  this->m_onRequest(frame);
}

#if HAS_WIFI

#ifdef ARDUINO
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

static bool isWifiConnected() {
  return WiFi.isConnected();
}

static IPAddress getLocalIP() {
  return WiFi.localIP();
}
#else
// ESP-IDF implementations
static uint8_t getSignalStrength() {
  wifi_ap_record_t ap_info;
  if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    int32_t dbm = ap_info.rssi;
    if (dbm > -63) return 4;
    if (dbm > -75) return 3;
    if (dbm > -88) return 2;
  }
  return 1;
}

static bool isWifiConnected() {
  wifi_ap_record_t ap_info;
  return esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK;
}

static IPAddress getLocalIP() {
  esp_netif_ip_info_t ip_info;
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
    return IPAddress(
      ip_info.ip.addr & 0xFF,
      (ip_info.ip.addr >> 8) & 0xFF,
      (ip_info.ip.addr >> 16) & 0xFF,
      (ip_info.ip.addr >> 24) & 0xFF
    );
  }
  return IPAddress();
}
#endif  // ARDUINO

#else  // !HAS_WIFI

static uint8_t getSignalStrength() { return 0; }
static bool isWifiConnected() { return false; }
static IPAddress getLocalIP() { return IPAddress(); }

#endif  // HAS_WIFI

void ApplianceBase::m_sendNetworkNotify(FrameType msgType) {
  NetworkNotifyData notify{};
  notify.setConnected(isWifiConnected());
  notify.setSignalStrength(getSignalStrength());
  notify.setIP(getLocalIP());
  notify.appendCRC();
  if (msgType == NETWORK_NOTIFY) {
    LOG_D(TAG, "Enqueuing a DEVICE_NETWORK(0x0D) notification...");
    this->m_queueNotify(msgType, std::move(notify));
  } else {
    LOG_D(TAG, "Answer to QUERY_NETWORK(0x63) request...");
    this->m_sendFrame(msgType, std::move(notify));
  }
}

void ApplianceBase::m_resetTimeout() {
  this->m_responseTimer.setCallback([this](Timer *timer) {
    LOG_D(TAG, "Response timeout...");
    if (!--this->m_remainAttempts) {
      if (this->m_request->onError != nullptr)
        this->m_request->onError();
      this->m_destroyRequest();
      return;
    }
    LOG_D(TAG, "Sending request again. Attempts left: %d...", this->m_remainAttempts);
    this->m_sendRequest(this->m_request);
    this->m_resetTimeout();
  });
  this->m_responseTimer.start(this->m_timeout);
}

void ApplianceBase::m_destroyRequest() {
  LOG_D(TAG, "Destroying the request...");
  this->m_responseTimer.stop();
  delete this->m_request;
  this->m_request = nullptr;
}

void ApplianceBase::m_sendFrame(FrameType type, const FrameData &data) {
  Frame frame(this->m_appType, this->m_protocol, type, data);
  LOG_D(TAG, "TX: %s", frame.toString().c_str());
  this->m_stream->write(frame.data(), frame.size());
  this->m_isBusy = true;
  this->m_periodTimer.setCallback([this](Timer *timer) {
    this->m_isBusy = false;
    timer->stop();
  });
  this->m_periodTimer.start(this->m_period);
}

void ApplianceBase::m_queueRequest(FrameType type, FrameData data, ResponseHandler onData, Handler onSuccess, Handler onError) {
  LOG_D(TAG, "Enqueuing the request...");
  this->m_queue.push_back(new Request{std::move(data), std::move(onData), std::move(onSuccess), std::move(onError), type});
}

void ApplianceBase::m_queueRequestPriority(FrameType type, FrameData data, ResponseHandler onData, Handler onSuccess, Handler onError) {
  LOG_D(TAG, "Priority request queuing...");
  this->m_queue.push_front(new Request{std::move(data), std::move(onData), std::move(onSuccess), std::move(onError), type});
}

void ApplianceBase::setBeeper(bool value) {
  LOG_D(TAG, "Turning %s beeper feedback...", value ? "ON" : "OFF");
  this->m_beeper = value;
}

} // namespace midea
} // namespace dudanov
