#include "gpio.h"

#include "bits.h"

static uint32_t port_index(const GPIO_TypeDef *port)
{
    /* GPIO ports are 0x400 apart starting at GPIOA. */
    return ((uint32_t)port - (uint32_t)GPIOA) / 0x400u;
}

void gpio_config(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode,
                 gpio_otype_t otype, gpio_pull_t pull, gpio_speed_t speed)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN << port_index(port);
    /* The STM32F4 errata sheet says to wait a couple of cycles after
     * enabling a peripheral clock before accessing it. A read-back works. */
    (void)RCC->AHB1ENR;

    /* field_set() is a pure function, so each line is one register read,
     * the new value computed in a CPU register, and one register write. */
    port->OTYPER  = field_set(port->OTYPER,  pin,      1, otype);
    port->OSPEEDR = field_set(port->OSPEEDR, pin * 2u, 2, speed);
    port->PUPDR   = field_set(port->PUPDR,   pin * 2u, 2, pull);
    port->MODER   = field_set(port->MODER,   pin * 2u, 2, mode); /* last: pin goes live */
}

void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af)
{
    /* AFR[0] holds pins 0-7, AFR[1] pins 8-15, 4 bits per pin. */
    port->AFR[pin >> 3] = field_set(port->AFR[pin >> 3], (pin & 7u) * 4u, 4, af);
}
