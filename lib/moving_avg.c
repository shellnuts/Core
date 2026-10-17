#include "moving_avg.h"

bool moving_avg_init(moving_avg_t *m, uint32_t n)
{
    /* TODO: validate n, then zero the state. */
    (void)m;
    (void)n;
    return false;
}

int32_t moving_avg_update(moving_avg_t *m, int32_t sample)
{
    /* TODO: subtract the sample falling out of the window (if the window
     * is full), store the new one, add it to the sum, return sum / count. */
    (void)m;
    (void)sample;
    return 0;
}
