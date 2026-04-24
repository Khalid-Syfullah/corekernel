#pragma once
#include <stdint.h>

#define PIT_HZ           1193182UL
#define TIMER_FREQ_HZ    100          /* 100 Hz = 10 ms per tick */

void timer_init(uint32_t hz);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
void timer_sleep(uint32_t ms);
