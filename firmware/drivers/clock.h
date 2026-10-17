#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>

/* Clock frequencies after clock_init(). Read these instead of hard-coding. */
extern uint32_t g_sysclk_hz; /* SYSCLK = HCLK (AHB, core, SysTick) */
extern uint32_t g_pclk1_hz;  /* APB1: USART2/3, I2C, TIM2-7,12-14   */
extern uint32_t g_pclk2_hz;  /* APB2: USART1/6, SPI1, ADC, TIM1/8   */

/*
 * Configure SYSCLK = 168 MHz from the PLL, AHB = 168, APB1 = 42, APB2 = 84 MHz.
 * Uses the board's 8 MHz HSE crystal; falls back to the internal 16 MHz HSI
 * if the crystal does not start. Returns false when the fallback was used.
 */
bool clock_init(void);

#endif
