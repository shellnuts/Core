#include <stdint.h>

#include "moving_avg.h"
#include "unit.h"

static void test_init_validates_n(void)
{
    moving_avg_t m;
    CHECK(!moving_avg_init(&m, 0));
    CHECK(!moving_avg_init(&m, MOVING_AVG_MAX_N + 1));
    CHECK(moving_avg_init(&m, 1));
    CHECK(moving_avg_init(&m, MOVING_AVG_MAX_N));
}

static void test_partial_window(void)
{
    moving_avg_t m;
    moving_avg_init(&m, 4);
    CHECK(moving_avg_update(&m, 10) == 10);  /* 10/1 */
    CHECK(moving_avg_update(&m, 20) == 15);  /* 30/2 */
    CHECK(moving_avg_update(&m, 0) == 10);   /* 30/3 */
}

static void test_full_window_slides(void)
{
    moving_avg_t m;
    moving_avg_init(&m, 4);
    moving_avg_update(&m, 1);
    moving_avg_update(&m, 2);
    moving_avg_update(&m, 3);
    CHECK(moving_avg_update(&m, 4) == 2);    /* (1+2+3+4)/4 = 2.5 -> 2 */
    CHECK(moving_avg_update(&m, 5) == 3);    /* (2+3+4+5)/4 = 3.5 -> 3 */
    CHECK(moving_avg_update(&m, 100) == 28); /* (3+4+5+100)/4 = 28 */
    /* Wrap the circular buffer several times. */
    for (int i = 0; i < 100; i++)
        moving_avg_update(&m, 7);
    CHECK(moving_avg_update(&m, 7) == 7);
}

static void test_truncates_toward_zero(void)
{
    moving_avg_t m;
    moving_avg_init(&m, 2);
    moving_avg_update(&m, -1);
    CHECK(moving_avg_update(&m, -2) == -1);  /* -3/2 = -1.5 -> -1 */
}

static void test_window_of_one(void)
{
    moving_avg_t m;
    moving_avg_init(&m, 1);
    CHECK(moving_avg_update(&m, 5) == 5);
    CHECK(moving_avg_update(&m, -9) == -9);
}

static void test_no_overflow_at_extremes(void)
{
    moving_avg_t m;
    moving_avg_init(&m, MOVING_AVG_MAX_N);
    int32_t avg = 0;
    for (int i = 0; i < 3 * MOVING_AVG_MAX_N; i++)
        avg = moving_avg_update(&m, INT32_MAX);
    CHECK(avg == INT32_MAX);
    for (int i = 0; i < 3 * MOVING_AVG_MAX_N; i++)
        avg = moving_avg_update(&m, INT32_MIN);
    CHECK(avg == INT32_MIN);
}

int main(void)
{
    RUN(test_init_validates_n);
    RUN(test_partial_window);
    RUN(test_full_window_slides);
    RUN(test_truncates_toward_zero);
    RUN(test_window_of_one);
    RUN(test_no_overflow_at_extremes);
    return UNIT_REPORT();
}
