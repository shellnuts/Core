#include "gpio.h"

static uint32_t port_index(const GPIO_TypeDef *port)
{
    /* GPIO ports are 0x400 apart starting at GPIOA. */
    return ((uint32_t)port - (uint32_t)GPIOA) / 0x400u;
}

/* Replace a 'width'-bit field at 'pos' with 'value'. */
static void set_field(volatile uint32_t *reg, uint32_t pos, uint32_t width, uint32_t value)
{
    uint32_t mask = ((1u << width) - 1u) << pos;
    *reg = (*reg & ~mask) | ((value << pos) & mask);
}

void gpio_config(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode,
                 gpio_otype_t otype, gpio_pull_t pull, gpio_speed_t speed)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN << port_index(port);
    /* The STM32F4 errata sheet says to wait a couple of cycles after
     * enabling a peripheral clock before accessing it. A read-back works. */
    (void)RCC->AHB1ENR;

    set_field(&port->OTYPER, pin, 1, otype);
    set_field(&port->OSPEEDR, pin * 2u, 2, speed);
    set_field(&port->PUPDR, pin * 2u, 2, pull);
    set_field(&port->MODER, pin * 2u, 2, mode);
}

void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af)
{
    set_field(&port->AFR[pin >> 3], (pin & 7u) * 4u, 4, af);
}
