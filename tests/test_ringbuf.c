#include "ringbuf.h"
#include "unit.h"

static void test_rejects_non_power_of_two(void)
{
    uint8_t s[6];
    ringbuf_t rb;
    CHECK(!ringbuf_init(&rb, s, sizeof s));
    CHECK(!ringbuf_init(&rb, s, 1));
}

static void test_empty_pop_fails(void)
{
    uint8_t s[8], b;
    ringbuf_t rb;
    ringbuf_init(&rb, s, sizeof s);
    CHECK(ringbuf_count(&rb) == 0);
    CHECK(!ringbuf_pop(&rb, &b));
}

static void test_fifo_order_and_full(void)
{
    uint8_t s[8], b;
    ringbuf_t rb;
    ringbuf_init(&rb, s, sizeof s);
    for (uint8_t i = 0; i < 8; i++)
        CHECK(ringbuf_push(&rb, i));
    CHECK(ringbuf_count(&rb) == 8);
    CHECK(!ringbuf_push(&rb, 99)); /* full: every slot usable */
    for (uint8_t i = 0; i < 8; i++) {
        CHECK(ringbuf_pop(&rb, &b));
        CHECK(b == i);
    }
    CHECK(!ringbuf_pop(&rb, &b));
}

static void test_counter_wraparound(void)
{
    uint8_t s[4], b;
    ringbuf_t rb;
    ringbuf_init(&rb, s, sizeof s);
    /* Start the counters just below 2^32 to exercise unsigned wrap. */
    atomic_store(&rb.head, 0xFFFFFFFEu);
    atomic_store(&rb.tail, 0xFFFFFFFEu);
    for (uint8_t i = 0; i < 4; i++)
        CHECK(ringbuf_push(&rb, (uint8_t)(10 + i)));
    CHECK(ringbuf_count(&rb) == 4);
    CHECK(!ringbuf_push(&rb, 0));
    for (uint8_t i = 0; i < 4; i++) {
        CHECK(ringbuf_pop(&rb, &b));
        CHECK(b == 10 + i);
    }
    CHECK(ringbuf_count(&rb) == 0);
}

int main(void)
{
    RUN(test_rejects_non_power_of_two);
    RUN(test_empty_pop_fails);
    RUN(test_fifo_order_and_full);
    RUN(test_counter_wraparound);
    return UNIT_REPORT();
}
