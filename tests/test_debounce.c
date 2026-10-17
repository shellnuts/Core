#include "debounce.h"
#include "unit.h"

#define STABLE 20u

/* Feed 'raw' once per ms from t0 for 'ms' ms. Counts events and records
 * when the last one happened. Returns the time after the last sample. */
typedef struct {
    int presses, releases;
    uint32_t last_event_time;
} tally_t;

static uint32_t feed(debounce_t *d, tally_t *t, bool raw, uint32_t t0, uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        uint32_t now = t0 + i;
        db_event_t e = debounce_update(d, raw, now);
        if (e == DB_EVENT_PRESS) {
            t->presses++;
            t->last_event_time = now;
        } else if (e == DB_EVENT_RELEASE) {
            t->releases++;
            t->last_event_time = now;
        }
    }
    return t0 + ms;
}

static void test_starts_released(void)
{
    debounce_t d;
    debounce_init(&d, STABLE);
    CHECK(d.state == DB_RELEASED);
    CHECK(!debounce_is_pressed(&d));
    CHECK(debounce_update(&d, false, 0) == DB_EVENT_NONE);
}

static void test_clean_press_exact_timing(void)
{
    debounce_t d;
    tally_t t = {0};
    debounce_init(&d, STABLE);
    uint32_t now = feed(&d, &t, false, 1000, 10);
    now = feed(&d, &t, true, now, 50);  /* raw goes high at t = 1010 */
    CHECK(t.presses == 1);
    CHECK(t.last_event_time == 1010 + STABLE);
    CHECK(debounce_is_pressed(&d));
    CHECK(d.state == DB_PRESSED);
}

static void test_short_glitch_ignored(void)
{
    debounce_t d;
    tally_t t = {0};
    debounce_init(&d, STABLE);
    uint32_t now = feed(&d, &t, true, 0, STABLE - 1); /* 19 ms: not enough */
    now = feed(&d, &t, false, now, 50);
    CHECK(t.presses == 0 && t.releases == 0);
    CHECK(!debounce_is_pressed(&d));
}

static void test_bouncy_press_and_release(void)
{
    debounce_t d;
    tally_t t = {0};
    debounce_init(&d, STABLE);
    uint32_t now = 0;
    /* Press: 10 ms of 2 ms bounces, then solid. */
    for (int i = 0; i < 5; i++) {
        now = feed(&d, &t, true, now, 1);
        now = feed(&d, &t, false, now, 1);
    }
    uint32_t solid = now;
    now = feed(&d, &t, true, now, 100);
    CHECK(t.presses == 1);
    CHECK(t.last_event_time == solid + STABLE);

    /* While MAYBE_RELEASED the debounced level is still "pressed". */
    now = feed(&d, &t, false, now, 5);
    CHECK(d.state == DB_MAYBE_RELEASED);
    CHECK(debounce_is_pressed(&d));
    now = feed(&d, &t, true, now, 1); /* bounce back */
    CHECK(d.state == DB_PRESSED);

    /* Release with bounces. */
    for (int i = 0; i < 5; i++) {
        now = feed(&d, &t, false, now, 1);
        now = feed(&d, &t, true, now, 1);
    }
    solid = now;
    now = feed(&d, &t, false, now, 100);
    CHECK(t.presses == 1);
    CHECK(t.releases == 1);
    CHECK(t.last_event_time == solid + STABLE);
    CHECK(!debounce_is_pressed(&d));
}

static void test_survives_millis_wraparound(void)
{
    debounce_t d;
    tally_t t = {0};
    debounce_init(&d, STABLE);
    /* Press starts 5 ms before the 32-bit counter wraps to 0. */
    uint32_t start = 0xFFFFFFFFu - 4u;
    feed(&d, &t, true, start, 50);
    CHECK(t.presses == 1);
    CHECK(t.last_event_time == (uint32_t)(start + STABLE));
}

int main(void)
{
    RUN(test_starts_released);
    RUN(test_clean_press_exact_timing);
    RUN(test_short_glitch_ignored);
    RUN(test_bouncy_press_and_release);
    RUN(test_survives_millis_wraparound);
    return UNIT_REPORT();
}
