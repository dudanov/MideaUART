#pragma once
#include <cstdint>
#include <functional>
#include <list>

namespace dudanov {

class Timer;
using TimerTick = unsigned long;
using TimerCallback = std::function<void(Timer *)>;
using Timers = std::list<Timer *>;

class TimerManager {
 public:
  static TimerTick ms() { return TimerManager::s_millis; }
  void registerTimer(Timer &timer) { m_timers.push_back(&timer); }
  void task();

 private:
  static TimerTick s_millis;
  Timers m_timers;
};

class Timer {
 public:
  Timer();
  bool isExpired() const { return TimerManager::ms() - this->m_last >= this->m_alarm; }
  bool isEnabled() const { return this->m_alarm; }
  void start(TimerTick ms) {
    this->m_alarm = ms;
    this->reset();
  }
  void stop() { this->m_alarm = 0; }
  void reset() { this->m_last = TimerManager::ms(); }
  void setCallback(TimerCallback cb) { this->m_callback = cb; }
  void call() { this->m_callback(this); }
 private:
  // Функция обратного вызова или лямбда
  TimerCallback m_callback;
  // Период срабатывания
  TimerTick m_alarm;
  // Последнее время срабатывания
  TimerTick m_last;
};


}  // namespace dudanov
