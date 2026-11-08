#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f407.h"

typedef enum { GPIO_MODE_INPUT = 0, GPIO_MODE_OUTPUT = 1, GPIO_MODE_AF = 2, GPIO_MODE_ANALOG = 3 } gpio_mode_t;
typedef enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 } gpio_pull_t;
typedef enum { GPIO_PUSH_PULL = 0, GPIO_OPEN_DRAIN = 1 } gpio_otype_t;
typedef enum { GPIO_SPEED_LOW = 0, GPIO_SPEED_MEDIUM = 1, GPIO_SPEED_FAST = 2, GPIO_SPEED_HIGH = 3 } gpio_speed_t;

/* Enables the port clock and configures one pin. */
void gpio_config(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode,
                 gpio_otype_t otype, gpio_pull_t pull, gpio_speed_t speed);

/* Selects alternate function 0..15 (see the datasheet's AF table). */
void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af);

/* BSRR writes are atomic: no read-modify-write, so they're ISR-safe. */
static inline void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool high)
{
    port->BSRR = high ? (1u << pin) : (1u << (pin + 16u));
}

static inline bool gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR >> pin) & 1u;
}

/* ODR read-modify-write: NOT safe if an ISR also writes this port. */
static inline void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    gpio_write(port, pin, !((port->ODR >> pin) & 1u));
}

#endif
