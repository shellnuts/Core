/*
 * lpf.h - First-order low-pass (exponential smoothing). PHASE 1, EXERCISE 3
 *
 *   y[k] = y[k-1] + alpha * (x[k] - y[k-1])
 *
 * This is the discrete version of an RC filter. With sample period
 * dt = 1/sample_hz and RC = 1 / (2*pi*cutoff_hz):
 *
 *   alpha = dt / (RC + dt)
 *
 * Requirements:
 *   - The FIRST sample after init sets y = x directly ("priming"), so the
 *     output doesn't crawl up from 0 when the sensor reads 3000 at power-on.
 *   - Use float, and float constants (3.14159265f, not 3.14159265): on a
 *     Cortex-M4F a double operation is done in slow software emulation.
 *     The firmware builds with -Wdouble-promotion to catch this.
 *
 * Questions to answer in docs/phase1/NOTES.md once the tests pass:
 *   1. What is alpha for cutoff 10 Hz at 1 kHz sampling?
 *   2. How many samples until a step input reaches 63% of its final value?
 *      How does that relate to RC?
 */
#ifndef LPF_H
#define LPF_H

#include <stdbool.h>

typedef struct {
    float alpha;
    float y;
    bool primed;
} lpf1_t;

void lpf1_init(lpf1_t *f, float cutoff_hz, float sample_hz);
float lpf1_update(lpf1_t *f, float x);

#endif
