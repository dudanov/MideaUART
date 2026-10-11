#pragma once
#include <cstdint>
extern uint32_t test_millis;
inline int64_t esp_timer_get_time() { return int64_t(test_millis) * 1000; }
