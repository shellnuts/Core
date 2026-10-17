/*
 * moving_avg.h - Moving-average (boxcar) filter.  PHASE 1, EXERCISE 2
 *
 * Averages the last N samples. Typical use: smoothing noisy ADC readings.
 *
 * Requirements:
 *   - O(1) per sample: keep a running sum, don't re-add all N samples.
 *   - Until N samples have arrived, average over the samples seen so far.
 *   - Result is the sum divided by the count using C integer division
 *     (truncates toward zero).
 *   - Must not overflow for ANY int32_t samples, including INT32_MAX.
 *     (Hint: what type must the running sum be?)
 *   - No malloc: storage lives inside the struct.
 *
 * You may change the struct fields if you prefer a different design.
 */
#ifndef MOVING_AVG_H
#define MOVING_AVG_H

#include <stdbool.h>
#include <stdint.h>

#define MOVING_AVG_MAX_N 64

typedef struct {
    int32_t samples[MOVING_AVG_MAX_N]; /* circular history */
    uint32_t n;                        /* window length */
    uint32_t idx;                      /* next slot to overwrite */
    uint32_t count;                    /* samples received, saturates at n */
    int64_t sum;                       /* running sum of the window */
} moving_avg_t;

/* Returns false (and leaves *m unusable) if n is 0 or > MOVING_AVG_MAX_N. */
bool moving_avg_init(moving_avg_t *m, uint32_t n);

/* Add a sample, return the current average. */
int32_t moving_avg_update(moving_avg_t *m, int32_t sample);

#endif
