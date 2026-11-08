#include "lpf.h"

void lpf1_init(lpf1_t *f, float cutoff_hz, float sample_hz)
{
    /* TODO: compute alpha (see lpf.h), clear y, mark not primed. */
    (void)f;
    (void)cutoff_hz;
    (void)sample_hz;
}

float lpf1_update(lpf1_t *f, float x)
{
    /* TODO: prime on the first sample, otherwise apply the filter. */
    (void)f;
    (void)x;
    return 0.0f;
}
