#pragma once
#include <Arduino.h>
#include <initializer_list>

namespace dudanov {

enum LogLevel : uint8_t {
  LOG_LEVEL_DEBUG,
};

template<typename T>
class Optional {
 public:
  Optional() = default;
  Optional(const T &value) : value_(value), hasValue_(true) {}
  void clear() { this->hasValue_ = false; }
  bool hasValue() const { return this->hasValue_; }
  T &value() { return this->value_; }
  const T &value() const { return this->value_; }
  bool operator==(const T &value) const {
    return this->hasValue_ && this->value_ == value;
  }
  bool operator!=(const T &value) const {
    return !this->hasValue_ || this->value_ != value;
  }
  friend bool operator!=(const T &value, const Optional<T> &opt) {
    return !opt.hasValue_ || opt.value_ != value;
  }
  bool hasUpdate(const T &value) const { return this->hasValue_ && this->value_ != value; }
 protected:
  T value_{};
  bool hasValue_{};
};

}  // namespace dudanov
