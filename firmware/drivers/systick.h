#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* 1 kHz system tick. Call after clock_init(). */
void systick_init(uint32_t core_hz);

/* Milliseconds since systick_init(); wraps after ~49.7 days.
 * Compare times with subtraction: (now - start) >= interval is wrap-safe. */
uint32_t millis(void);

void delay_ms(uint32_t ms);

#endif
