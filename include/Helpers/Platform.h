#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#include <IPAddress.h>
#else
// ESP-IDF compatibility layer
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <functional>

#include "esp_timer.h"
#include "esp_random.h"

// millis() replacement
inline unsigned long millis() {
  return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

// random() replacement
inline long random(long max) {
  return esp_random() % max;
}

inline long random(long min, long max) {
  return min + (esp_random() % (max - min));
}

// String replacement - use std::string
using String = std::string;

// F() macro - no-op on ESP-IDF (no flash strings needed)
#define F(x) x

// __FlashStringHelper - just const char* on ESP-IDF
using __FlashStringHelper = const char;

// Stream interface for ESP-IDF
class Stream {
 public:
  virtual ~Stream() = default;
  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;
  virtual size_t write(uint8_t data) = 0;
  virtual size_t write(const uint8_t *data, size_t size) = 0;
  virtual void flush() = 0;
};

// IPAddress for ESP-IDF
class IPAddress {
 public:
  IPAddress() : addr_{0, 0, 0, 0} {}
  IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : addr_{a, b, c, d} {}
  uint8_t operator[](int index) const { return addr_[index]; }
 private:
  uint8_t addr_[4];
};

#endif  // ARDUINO

// WiFi availability detection
#ifdef ARDUINO_ARCH_ESP8266
  // ESP8266 always has WiFi
  #define HAS_WIFI 1
#elif defined(ESP32)
  // ESP32 family: check chip capabilities
  #include "soc/soc_caps.h"
  #if SOC_WIFI_SUPPORTED
    #define HAS_WIFI 1
  #else
    #define HAS_WIFI 0
  #endif
#else
  #define HAS_WIFI 0
#endif
