/*
 * stm32f407.h - Hand-written minimal register map for the STM32F407.
 *
 * Every address and offset here comes from RM0090 (STM32F4 reference manual)
 * and PM0214 (Cortex-M4 programming manual). Writing this yourself is the
 * point: once you can read the manual and map a peripheral, ST's CMSIS
 * header (stm32f407xx.h) holds no secrets. Switch to CMSIS later.
 *
 * The _Static_asserts check the struct layouts against the manual's offset
 * tables, so a missing padding word fails the build instead of the board.
 */
#ifndef STM32F407_H
#define STM32F407_H

#include <stddef.h>
#include <stdint.h>

#define __IO volatile

/* ---------------------------------------------------------------- RCC --- */
typedef struct {
    __IO uint32_t CR;          /* 0x00 */
    __IO uint32_t PLLCFGR;     /* 0x04 */
    __IO uint32_t CFGR;        /* 0x08 */
    __IO uint32_t CIR;         /* 0x0C */
    __IO uint32_t AHB1RSTR;    /* 0x10 */
    __IO uint32_t AHB2RSTR;    /* 0x14 */
    __IO uint32_t AHB3RSTR;    /* 0x18 */
    uint32_t      RESERVED0;   /* 0x1C */
    __IO uint32_t APB1RSTR;    /* 0x20 */
    __IO uint32_t APB2RSTR;    /* 0x24 */
    uint32_t      RESERVED1[2];
    __IO uint32_t AHB1ENR;     /* 0x30 */
    __IO uint32_t AHB2ENR;     /* 0x34 */
    __IO uint32_t AHB3ENR;     /* 0x38 */
    uint32_t      RESERVED2;
    __IO uint32_t APB1ENR;     /* 0x40 */
    __IO uint32_t APB2ENR;     /* 0x44 */
    uint32_t      RESERVED3[2];
    __IO uint32_t AHB1LPENR;   /* 0x50 */
    __IO uint32_t AHB2LPENR;   /* 0x54 */
    __IO uint32_t AHB3LPENR;   /* 0x58 */
    uint32_t      RESERVED4;
    __IO uint32_t APB1LPENR;   /* 0x60 */
    __IO uint32_t APB2LPENR;   /* 0x64 */
    uint32_t      RESERVED5[2];
    __IO uint32_t BDCR;        /* 0x70 */
    __IO uint32_t CSR;         /* 0x74 */
    uint32_t      RESERVED6[2];
    __IO uint32_t SSCGR;       /* 0x80 */
    __IO uint32_t PLLI2SCFGR;  /* 0x84 */
} RCC_TypeDef;
_Static_assert(offsetof(RCC_TypeDef, AHB1ENR) == 0x30, "RCC layout");
_Static_assert(offsetof(RCC_TypeDef, APB2ENR) == 0x44, "RCC layout");
_Static_assert(offsetof(RCC_TypeDef, PLLI2SCFGR) == 0x84, "RCC layout");

#define RCC_CR_HSION        (1u << 0)
#define RCC_CR_HSIRDY       (1u << 1)
#define RCC_CR_HSEON        (1u << 16)
#define RCC_CR_HSERDY       (1u << 17)
#define RCC_CR_PLLON        (1u << 24)
#define RCC_CR_PLLRDY       (1u << 25)

#define RCC_PLLCFGR_PLLM_Pos   0
#define RCC_PLLCFGR_PLLN_Pos   6
#define RCC_PLLCFGR_PLLP_Pos   16
#define RCC_PLLCFGR_PLLSRC_HSE (1u << 22)
#define RCC_PLLCFGR_PLLQ_Pos   24

#define RCC_CFGR_SW_Msk     (3u << 0)
#define RCC_CFGR_SW_PLL     (2u << 0)
#define RCC_CFGR_SWS_Msk    (3u << 2)
#define RCC_CFGR_SWS_PLL    (2u << 2)
#define RCC_CFGR_HPRE_Pos   4
#define RCC_CFGR_PPRE1_Pos  10
#define RCC_CFGR_PPRE2_Pos  13

#define RCC_AHB1ENR_GPIOAEN (1u << 0)   /* GPIOxEN = bit x, A..I */
#define RCC_APB1ENR_PWREN   (1u << 28)
#define RCC_APB2ENR_USART1EN (1u << 4)

/* -------------------------------------------------------------- FLASH --- */
typedef struct {
    __IO uint32_t ACR;     /* 0x00 */
    __IO uint32_t KEYR;    /* 0x04 */
    __IO uint32_t OPTKEYR; /* 0x08 */
    __IO uint32_t SR;      /* 0x0C */
    __IO uint32_t CR;      /* 0x10 */
    __IO uint32_t OPTCR;   /* 0x14 */
} FLASH_TypeDef;

#define FLASH_ACR_LATENCY_Msk (7u << 0)
#define FLASH_ACR_PRFTEN      (1u << 8)
#define FLASH_ACR_ICEN        (1u << 9)
#define FLASH_ACR_DCEN        (1u << 10)

/* ---------------------------------------------------------------- PWR --- */
typedef struct {
    __IO uint32_t CR;  /* 0x00 */
    __IO uint32_t CSR; /* 0x04 */
} PWR_TypeDef;

#define PWR_CR_VOS (1u << 14)

/* --------------------------------------------------------------- GPIO --- */
typedef struct {
    __IO uint32_t MODER;   /* 0x00 */
    __IO uint32_t OTYPER;  /* 0x04 */
    __IO uint32_t OSPEEDR; /* 0x08 */
    __IO uint32_t PUPDR;   /* 0x0C */
    __IO uint32_t IDR;     /* 0x10 */
    __IO uint32_t ODR;     /* 0x14 */
    __IO uint32_t BSRR;    /* 0x18 */
    __IO uint32_t LCKR;    /* 0x1C */
    __IO uint32_t AFR[2];  /* 0x20, 0x24 */
} GPIO_TypeDef;
_Static_assert(offsetof(GPIO_TypeDef, AFR) == 0x20, "GPIO layout");

/* -------------------------------------------------------------- USART --- */
typedef struct {
    __IO uint32_t SR;   /* 0x00 */
    __IO uint32_t DR;   /* 0x04 */
    __IO uint32_t BRR;  /* 0x08 */
    __IO uint32_t CR1;  /* 0x0C */
    __IO uint32_t CR2;  /* 0x10 */
    __IO uint32_t CR3;  /* 0x14 */
    __IO uint32_t GTPR; /* 0x18 */
} USART_TypeDef;

#define USART_SR_ORE    (1u << 3)
#define USART_SR_RXNE   (1u << 5)
#define USART_SR_TC     (1u << 6)
#define USART_SR_TXE    (1u << 7)
#define USART_CR1_RE    (1u << 2)
#define USART_CR1_TE    (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE    (1u << 13)

/* ------------------------------------------------- Cortex-M4 core (PM0214) */
typedef struct {
    __IO uint32_t CTRL;  /* 0x00 */
    __IO uint32_t LOAD;  /* 0x04 */
    __IO uint32_t VAL;   /* 0x08 */
    __IO uint32_t CALIB; /* 0x0C */
} SysTick_TypeDef;

#define SysTick_CTRL_ENABLE    (1u << 0)
#define SysTick_CTRL_TICKINT   (1u << 1)
#define SysTick_CTRL_CLKSOURCE (1u << 2)   /* 1 = processor clock */

typedef struct {
    __IO uint32_t ISER[8];       /* 0x000 set-enable */
    uint32_t      RESERVED0[24];
    __IO uint32_t ICER[8];       /* 0x080 clear-enable */
    uint32_t      RESERVED1[24];
    __IO uint32_t ISPR[8];       /* 0x100 set-pending */
    uint32_t      RESERVED2[24];
    __IO uint32_t ICPR[8];       /* 0x180 clear-pending */
    uint32_t      RESERVED3[24];
    __IO uint32_t IABR[8];       /* 0x200 active */
    uint32_t      RESERVED4[56];
    __IO uint8_t  IP[240];       /* 0x300 priority, one byte per IRQ */
} NVIC_TypeDef;
_Static_assert(offsetof(NVIC_TypeDef, IP) == 0x300, "NVIC layout");

#define SCB_CPACR (*(__IO uint32_t *)0xE000ED88u) /* FPU access control */

/* ------------------------------------------------------- Memory map --- */
#define PERIPH_BASE   0x40000000u
#define APB1_BASE     (PERIPH_BASE + 0x00000u)
#define APB2_BASE     (PERIPH_BASE + 0x10000u)
#define AHB1_BASE     (PERIPH_BASE + 0x20000u)

#define PWR      ((PWR_TypeDef *)(APB1_BASE + 0x7000u))
#define USART1   ((USART_TypeDef *)(APB2_BASE + 0x1000u))
#define GPIOA    ((GPIO_TypeDef *)(AHB1_BASE + 0x0000u))
#define GPIOB    ((GPIO_TypeDef *)(AHB1_BASE + 0x0400u))
#define GPIOC    ((GPIO_TypeDef *)(AHB1_BASE + 0x0800u))
#define GPIOD    ((GPIO_TypeDef *)(AHB1_BASE + 0x0C00u))
#define GPIOE    ((GPIO_TypeDef *)(AHB1_BASE + 0x1000u))
#define GPIOF    ((GPIO_TypeDef *)(AHB1_BASE + 0x1400u))
#define GPIOG    ((GPIO_TypeDef *)(AHB1_BASE + 0x1800u))
#define RCC      ((RCC_TypeDef *)(AHB1_BASE + 0x3800u))
#define FLASH_R  ((FLASH_TypeDef *)(AHB1_BASE + 0x3C00u))

#define SysTick  ((SysTick_TypeDef *)0xE000E010u)
#define NVIC     ((NVIC_TypeDef *)0xE000E100u)

/* IRQ numbers (position in the vector table after the 16 core entries) */
#define USART1_IRQn 37

static inline void nvic_enable_irq(unsigned irqn, uint8_t priority)
{
    /* The F4 implements the top 4 priority bits; lower number = more urgent. */
    NVIC->IP[irqn] = (uint8_t)(priority << 4);
    NVIC->ISER[irqn >> 5] = 1u << (irqn & 31u);
}

static inline void __disable_irq(void) { __asm volatile("cpsid i" ::: "memory"); }
static inline void __enable_irq(void)  { __asm volatile("cpsie i" ::: "memory"); }
static inline void __DSB(void) { __asm volatile("dsb 0xF" ::: "memory"); }
static inline void __ISB(void) { __asm volatile("isb 0xF" ::: "memory"); }
static inline void __WFI(void) { __asm volatile("wfi"); }

#endif /* STM32F407_H */
