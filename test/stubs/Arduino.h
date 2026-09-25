#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

class __FlashStringHelper;
class String : public std::string {
 public:
  using std::string::string;
  String(const std::string &value) : std::string(value) {}
  String(const __FlashStringHelper *value) : std::string(reinterpret_cast<const char *>(value)) {}
};
#define F(x) reinterpret_cast<const __FlashStringHelper *>(x)
#define PROGMEM
#define pgm_read_byte(p) (*(p))

extern unsigned long test_millis;
inline unsigned long millis() { return test_millis; }
inline long random(long maximum) { return maximum / 2; }

class Stream {
 public:
  virtual ~Stream() = default;
  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *, size_t) = 0;
  virtual void flush() = 0;
};
