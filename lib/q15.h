/*
 * q15.h - Q15 fixed-point arithmetic.           PHASE 1, EXERCISE 5
 *
 * Q15 stores a number in [-1, 1) in an int16_t: value = raw / 32768.
 *   0x7FFF = +0.99997   0x4000 = +0.5   0x0000 = 0   0x8000 = -1.0
 *
 * Why bother when the F407 has an FPU? Many MCUs (Cortex-M0/M3) don't;
 * CMSIS-DSP and audio codecs use Q15/Q31; and fixed point teaches you
 * where precision and overflow actually come from.
 *
 * Requirements:
 *   - Saturate instead of wrapping: 0.75 + 0.75 = 0x7FFF, not -0.5.
 *   - q15_mul rounds to nearest. Watch for -1 * -1 = +1, which doesn't fit.
 *   - Use a wider type (int32_t) for intermediate results.
 *   - Right-shifting a negative number is implementation-defined in C
 *     (arithmetic on GCC/ARM). That's acceptable here; know that it's there.
 */
#ifndef Q15_H
#define Q15_H

#include <stdbool.h>
#include <stdint.h>

typedef int16_t q15_t;

#define Q15_MAX ((q15_t)0x7FFF)
#define Q15_MIN ((q15_t)-32768)

/* Round to nearest; values outside [-1, 1) saturate. */
q15_t q15_from_float(float x);
float q15_to_float(q15_t x);

q15_t q15_sat(int32_t x);              /* clamp an int32 into q15 range */
q15_t q15_add(q15_t a, q15_t b);       /* saturating */
q15_t q15_sub(q15_t a, q15_t b);       /* saturating */
q15_t q15_mul(q15_t a, q15_t b);       /* rounded, saturating */

/* Fixed-point version of lpf1 (lpf.h). alpha is given directly in Q15.
 * First sample primes y = x, exactly like the float version.
 * Careful: (x - y) can be anywhere in (-2, 2), which does NOT fit in a q15. */
typedef struct {
    q15_t alpha;
    q15_t y;
    bool primed;
} lpf1_q15_t;

void lpf1_q15_init(lpf1_q15_t *f, q15_t alpha);
q15_t lpf1_q15_update(lpf1_q15_t *f, q15_t x);

#endif
