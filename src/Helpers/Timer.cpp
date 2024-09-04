#include <Arduino.h>
#include "Helpers/Timer.h"

namespace dudanov {

TimerTick TimerManager::s_millis;

// Dummy function for incorrect using case.
static void dummy(Timer *timer) { timer->stop(); }
Timer::Timer() : m_callback(dummy), m_alarm(0) {}

/// Timers task. Must be periodically called in loop function.
void TimerManager::task() {
  s_millis = ::millis();

  for (auto timer : m_timers) {
    if (timer->isEnabled() && timer->isExpired())
      timer->call();
  }
}

}  // namespace dudanov
