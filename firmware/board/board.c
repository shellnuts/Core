#include "board.h"

#include "gpio.h"

/* LEDs: cathode to the MCU pin, so they light when the pin is LOW. */
#define LED0_PORT GPIOF
#define LED0_PIN  9
#define LED1_PORT GPIOF
#define LED1_PIN  10

/* Buzzer driven through a transistor: HIGH = on. */
#define BEEP_PORT GPIOF
#define BEEP_PIN  8

typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
    bool active_high;
} key_def_t;

/* KEY0..2 short to GND (need pull-up), WK_UP shorts to 3.3 V (needs pull-down). */
static const key_def_t s_keys[KEY_COUNT] = {
    [KEY0] = {GPIOE, 4, false},
    [KEY1] = {GPIOE, 3, false},
    [KEY2] = {GPIOE, 2, false},
    [KEY_WKUP] = {GPIOA, 0, true},
};

void board_init(void)
{
    gpio_config(LED0_PORT, LED0_PIN, GPIO_MODE_OUTPUT, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_SPEED_LOW);
    gpio_config(LED1_PORT, LED1_PIN, GPIO_MODE_OUTPUT, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_SPEED_LOW);
    board_led_set(LED0, false);
    board_led_set(LED1, false);

    gpio_config(BEEP_PORT, BEEP_PIN, GPIO_MODE_OUTPUT, GPIO_PUSH_PULL, GPIO_PULL_DOWN, GPIO_SPEED_LOW);
    board_beep(false);

    for (int k = 0; k < KEY_COUNT; k++) {
        const key_def_t *d = &s_keys[k];
        gpio_config(d->port, d->pin, GPIO_MODE_INPUT, GPIO_PUSH_PULL,
                    d->active_high ? GPIO_PULL_DOWN : GPIO_PULL_UP, GPIO_SPEED_LOW);
    }
}

void board_led_set(board_led_t led, bool on)
{
    if (led == LED0)
        gpio_write(LED0_PORT, LED0_PIN, !on);
    else
        gpio_write(LED1_PORT, LED1_PIN, !on);
}

void board_led_toggle(board_led_t led)
{
    if (led == LED0)
        gpio_toggle(LED0_PORT, LED0_PIN);
    else
        gpio_toggle(LED1_PORT, LED1_PIN);
}

void board_beep(bool on)
{
    gpio_write(BEEP_PORT, BEEP_PIN, on);
}

bool board_key_pressed(board_key_t key)
{
    const key_def_t *d = &s_keys[key];
    return gpio_read(d->port, d->pin) == d->active_high;
}
