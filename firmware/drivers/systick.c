#include "systick.h"

#include "stm32f407.h"

/* volatile: written in an ISR, read in thread code. Without it the compiler
 * may cache the value in a register and delay_ms() would spin forever. */
static volatile uint32_t s_ticks;

void systick_init(uint32_t core_hz)
{
    SysTick->LOAD = core_hz / 1000u - 1u; /* 24-bit reload */
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE | SysTick_CTRL_TICKINT | SysTick_CTRL_ENABLE;
}

void SysTick_Handler(void)
{
    s_ticks++;
}

uint32_t millis(void)
{
    return s_ticks; /* aligned 32-bit read is atomic on Cortex-M */
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms)
        __WFI(); /* sleep until the next interrupt instead of burning power */
}
