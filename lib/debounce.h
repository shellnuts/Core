/*
 * debounce.h - Push-button debouncer as a state machine. PHASE 1, EXERCISE 4
 *
 * Mechanical contacts bounce: one press looks like 1-10 ms of random
 * 0/1 flicker. This debouncer only accepts a new level once the raw input
 * has held it for 'stable_ms' milliseconds.
 *
 *             raw pressed                     held >= stable_ms
 *   RELEASED ------------> MAYBE_PRESSED -----------------------> PRESSED
 *       ^                       |                  (emit DB_EVENT_PRESS)
 *       +---- raw released -----+
 *
 *   ...and the mirror image for PRESSED -> MAYBE_RELEASED -> RELEASED
 *   (emit DB_EVENT_RELEASE).
 *
 * Exact timing rule (the tests check it): entering a MAYBE_ state records
 * since = now_ms. The event fires on the first update where the raw level
 * still agrees AND (now_ms - since) >= stable_ms.
 *
 * 'now_ms' comes from millis(), which wraps from 0xFFFFFFFF to 0 after
 * 49.7 days. Your code must keep working across the wrap: the tests
 * check this.
 */
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DB_RELEASED,
    DB_MAYBE_PRESSED,
    DB_PRESSED,
    DB_MAYBE_RELEASED,
} db_state_t;

typedef enum {
    DB_EVENT_NONE,
    DB_EVENT_PRESS,
    DB_EVENT_RELEASE,
} db_event_t;

typedef struct {
    db_state_t state;
    uint32_t stable_ms;
    uint32_t since;
} debounce_t;

/* Starts in DB_RELEASED. */
void debounce_init(debounce_t *d, uint32_t stable_ms);

/* Call periodically (e.g. every 1-5 ms) with the raw, active-high level. */
db_event_t debounce_update(debounce_t *d, bool raw_pressed, uint32_t now_ms);

/* Debounced level: true in PRESSED and MAYBE_RELEASED. */
bool debounce_is_pressed(const debounce_t *d);

#endif
