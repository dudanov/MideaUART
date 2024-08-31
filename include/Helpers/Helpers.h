#pragma once
#include <Arduino.h>

namespace dudanov {

class ByteHelpers {
 public:
  template<typename T> static T getLE(const uint8_t *src) {
    T dst = 0;
    for (size_t idx = 0; idx < sizeof(T); ++idx, ++src)
      dst |= static_cast<T>(*src) << (idx * 8);
    return dst;
  }
};

template<typename T> class Optional {
 public:
  Optional() = default;
  Optional(const T &value) : m_value(value), m_hasValue(true) {}
  void clear() { m_hasValue = false; }
  bool hasValue() const { return m_hasValue; }
  T &value() { return m_value; }
  const T &value() const { return m_value; }
  bool operator==(const T &value) const { return m_hasValue && m_value == value; }
  bool operator!=(const T &value) const { return !m_hasValue || m_value != value; }
  friend bool operator!=(const T &value, const Optional<T> &opt) { return !opt.m_hasValue || opt.m_value != value; }
  bool hasUpdate(const T &value) const { return m_hasValue && m_value != value; }

 protected:
  T m_value{};
  bool m_hasValue{};
};

}  // namespace dudanov
