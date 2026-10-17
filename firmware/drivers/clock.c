#include "clock.h"

#include "stm32f407.h"

uint32_t g_sysclk_hz = 16000000u; /* reset default: HSI */
uint32_t g_pclk1_hz = 16000000u;
uint32_t g_pclk2_hz = 16000000u;

static bool wait_flag(volatile uint32_t *reg, uint32_t mask, uint32_t timeout)
{
    while ((*reg & mask) == 0) {
        if (timeout-- == 0)
            return false;
    }
    return true;
}

bool clock_init(void)
{
    /* Core voltage scale 1 is required for 168 MHz. */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    /* Try the 8 MHz crystal first. */
    RCC->CR |= RCC_CR_HSEON;
    bool hse_ok = wait_flag(&RCC->CR, RCC_CR_HSERDY, 500000u);

    /*
     * PLL: f_vco = f_in / M * N, SYSCLK = f_vco / P, USB/SDIO = f_vco / Q.
     * Keep f_in / M = 1 MHz (the manual recommends 2 MHz for jitter; 1 MHz
     * makes the arithmetic obvious), f_vco = 336 MHz, P = 2 -> 168 MHz,
     * Q = 7 -> 48 MHz (needed by USB later).
     */
    uint32_t pllm = hse_ok ? 8u : 16u;
    uint32_t pllcfgr = (pllm << RCC_PLLCFGR_PLLM_Pos)
                     | (336u << RCC_PLLCFGR_PLLN_Pos)
                     | (0u << RCC_PLLCFGR_PLLP_Pos)   /* 00 => P = 2 */
                     | (7u << RCC_PLLCFGR_PLLQ_Pos);
    if (hse_ok)
        pllcfgr |= RCC_PLLCFGR_PLLSRC_HSE;
    else
        RCC->CR &= ~RCC_CR_HSEON;
    RCC->PLLCFGR = pllcfgr;

    /* Bus prescalers: AHB /1, APB1 /4 (max 42 MHz), APB2 /2 (max 84 MHz). */
    RCC->CFGR = (RCC->CFGR & ~((0xFu << RCC_CFGR_HPRE_Pos)
                             | (7u << RCC_CFGR_PPRE1_Pos)
                             | (7u << RCC_CFGR_PPRE2_Pos)))
              | (0u << RCC_CFGR_HPRE_Pos)
              | (5u << RCC_CFGR_PPRE1_Pos)  /* 101 => /4 */
              | (4u << RCC_CFGR_PPRE2_Pos); /* 100 => /2 */

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0) {
    }

    /* Flash can't keep up with 168 MHz: 5 wait states at 2.7-3.6 V, plus
     * prefetch and the instruction/data caches (ART accelerator). Must be
     * set BEFORE switching to the faster clock. */
    FLASH_R->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | 5u;
    while ((FLASH_R->ACR & FLASH_ACR_LATENCY_Msk) != 5u) {
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL) {
    }

    g_sysclk_hz = 168000000u;
    g_pclk1_hz = 42000000u;
    g_pclk2_hz = 84000000u;
    return hse_ok;
}
