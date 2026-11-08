#include "uart.h"

#include "clock.h"
#include "gpio.h"
#include "ringbuf.h"
#include "stm32f407.h"

static uint8_t s_rx_storage[256];
static ringbuf_t s_rx;

void uart1_init(uint32_t baud)
{
    ringbuf_init(&s_rx, s_rx_storage, sizeof s_rx_storage);

    /* PA9 = USART1_TX, PA10 = USART1_RX, both alternate function 7. */
    gpio_config(GPIOA, 9, GPIO_MODE_AF, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_SPEED_FAST);
    gpio_config(GPIOA, 10, GPIO_MODE_AF, GPIO_PUSH_PULL, GPIO_PULL_UP, GPIO_SPEED_FAST);
    gpio_set_af(GPIOA, 9, 7);
    gpio_set_af(GPIOA, 10, 7);

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;

    /* With 16x oversampling BRR = f_pclk / baud as a 12.4 fixed-point number,
     * which is just the rounded integer division. 84 MHz / 115200 = 729.2. */
    USART1->BRR = (g_pclk2_hz + baud / 2u) / baud;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    nvic_enable_irq(USART1_IRQn, 6);
}

void uart1_putc(char c)
{
    while ((USART1->SR & USART_SR_TXE) == 0) {
    }
    USART1->DR = (uint8_t)c;
}

void uart1_write(const char *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n')
            uart1_putc('\r'); /* terminals expect CRLF */
        uart1_putc(data[i]);
    }
}

bool uart1_getc(char *c)
{
    uint8_t b;
    if (!ringbuf_pop(&s_rx, &b))
        return false;
    *c = (char)b;
    return true;
}

void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;
    /* Reading SR then DR clears RXNE and the ORE overrun flag. If ORE is
     * left set the interrupt keeps firing and the main loop starves. */
    if (sr & (USART_SR_RXNE | USART_SR_ORE)) {
        uint8_t b = (uint8_t)USART1->DR;
        if (sr & USART_SR_RXNE)
            (void)ringbuf_push(&s_rx, b); /* drop the byte if the buffer is full */
    }
}
