#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * USART1 on PA9 (TX) / PA10 (RX), 8N1. On the Explorer board this is wired
 * to the CH340 USB-serial chip on the "USB_232" connector.
 * TX is polled (simple, blocking); RX is interrupt-driven into a ring buffer.
 * Exercise (phase 3): make TX interrupt- or DMA-driven too.
 */
void uart1_init(uint32_t baud);
void uart1_putc(char c);
void uart1_write(const char *data, size_t len);
bool uart1_getc(char *c); /* non-blocking; false if nothing received */

#endif
