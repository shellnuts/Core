#include "debounce.h"

void debounce_init(debounce_t *d, uint32_t stable_ms)
{
    /* TODO */
    (void)d;
    (void)stable_ms;
}

db_event_t debounce_update(debounce_t *d, bool raw_pressed, uint32_t now_ms)
{
    /* TODO: a switch on d->state with one case per state.
     * Compare times as (now_ms - d->since) >= d->stable_ms, never as
     * now_ms >= d->since + d->stable_ms (why? see debounce.h). */
    (void)d;
    (void)raw_pressed;
    (void)now_ms;
    return DB_EVENT_NONE;
}

bool debounce_is_pressed(const debounce_t *d)
{
    /* TODO */
    (void)d;
    return false;
}
