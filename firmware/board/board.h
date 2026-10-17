/*
 * board.h - ALIENTEK Explorer (探索者) STM32F407ZGT6 board support.
 *
 * Pin assignments come from the ALIENTEK schematic. If something doesn't
 * respond, check your board revision's schematic first (docs/BOARD.md).
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

typedef enum { LED0 = 0, LED1 = 1 } board_led_t;                /* DS0 red, DS1 green */
typedef enum { KEY0 = 0, KEY1, KEY2, KEY_WKUP, KEY_COUNT } board_key_t;

void board_init(void);

void board_led_set(board_led_t led, bool on);
void board_led_toggle(board_led_t led);
void board_beep(bool on);

/* Raw level, already converted to "pressed = true". Not debounced.
 * Exercise (phase 1/2): write a debouncer as a small state machine. */
bool board_key_pressed(board_key_t key);

#endif
