#include "q15.h"

q15_t q15_sat(int32_t x)
{
    /* TODO */
    (void)x;
    return 0;
}

q15_t q15_from_float(float x)
{
    /* TODO: scale by 32768, round to nearest, saturate.
     * Hint: saturate BEFORE converting to an integer type: converting an
     * out-of-range float to int is undefined behaviour. */
    (void)x;
    return 0;
}

float q15_to_float(q15_t x)
{
    /* TODO */
    (void)x;
    return 0.0f;
}

q15_t q15_add(q15_t a, q15_t b)
{
    /* TODO */
    (void)a;
    (void)b;
    return 0;
}

q15_t q15_sub(q15_t a, q15_t b)
{
    /* TODO */
    (void)a;
    (void)b;
    return 0;
}

q15_t q15_mul(q15_t a, q15_t b)
{
    /* TODO: the int32 product is Q30. Add half an LSB (1 << 14) to round,
     * shift right by 15, then saturate. */
    (void)a;
    (void)b;
    return 0;
}

void lpf1_q15_init(lpf1_q15_t *f, q15_t alpha)
{
    /* TODO */
    (void)f;
    (void)alpha;
}

q15_t lpf1_q15_update(lpf1_q15_t *f, q15_t x)
{
    /* TODO: same algorithm as lpf1_update, in integer arithmetic. */
    (void)f;
    (void)x;
    return 0;
}
