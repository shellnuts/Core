#include "lpf.h"
#include "q15.h"
#include "unit.h"

static void test_conversions(void)
{
    CHECK(q15_from_float(0.0f) == 0);
    CHECK(q15_from_float(0.5f) == 0x4000);
    CHECK(q15_from_float(-0.5f) == -0x4000);
    CHECK(q15_from_float(-1.0f) == Q15_MIN);
    CHECK(q15_from_float(1.0f) == Q15_MAX);   /* +1 doesn't fit: saturate */
    CHECK(q15_from_float(123.0f) == Q15_MAX);
    CHECK(q15_from_float(-123.0f) == Q15_MIN);
    CHECK(q15_from_float(1e30f) == Q15_MAX);  /* no UB on huge values */
    CHECK(q15_from_float(0.1f) == 3277);      /* 3276.8 rounds up */
    CHECK(q15_from_float(-0.1f) == -3277);
    CHECK_NEAR(q15_to_float(0x4000), 0.5, 1e-9);
    CHECK_NEAR(q15_to_float(Q15_MIN), -1.0, 1e-9);
    CHECK_NEAR(q15_to_float(Q15_MAX), 32767.0 / 32768.0, 1e-9);
}

static void test_sat(void)
{
    CHECK(q15_sat(0) == 0);
    CHECK(q15_sat(32767) == Q15_MAX);
    CHECK(q15_sat(32768) == Q15_MAX);
    CHECK(q15_sat(1000000) == Q15_MAX);
    CHECK(q15_sat(-32768) == Q15_MIN);
    CHECK(q15_sat(-32769) == Q15_MIN);
}

static void test_add_sub_saturate(void)
{
    q15_t a = q15_from_float(0.75f);
    CHECK(q15_add(a, a) == Q15_MAX);                  /* not -0.5! */
    CHECK(q15_add(-a, -a) == Q15_MIN);
    CHECK(q15_add(0x1000, 0x2000) == 0x3000);
    CHECK(q15_sub(Q15_MIN, 1) == Q15_MIN);
    CHECK(q15_sub(Q15_MAX, -1) == Q15_MAX);
    CHECK(q15_sub(0x3000, 0x1000) == 0x2000);
}

static void test_mul(void)
{
    CHECK(q15_mul(0x4000, 0x4000) == 0x2000);         /* 0.5 * 0.5 = 0.25 */
    CHECK(q15_mul(0x4000, -0x4000) == -0x2000);
    CHECK(q15_mul(Q15_MAX, 0x4000) == 0x4000);        /* 0.99997*0.5 rounds to 0.5 */
    CHECK(q15_mul(Q15_MIN, Q15_MIN) == Q15_MAX);      /* -1 * -1 = +1 saturates */
    CHECK(q15_mul(Q15_MIN, Q15_MAX) == -32767);
    CHECK(q15_mul(1, 0x4000) == 1);                   /* 0.5 LSB rounds up */
    CHECK(q15_mul(0, Q15_MIN) == 0);
}

static void test_lpf_q15_primes(void)
{
    lpf1_q15_t f;
    lpf1_q15_init(&f, q15_from_float(0.0591174f));
    CHECK(lpf1_q15_update(&f, 0x2000) == 0x2000);
}

static void test_lpf_q15_full_scale_swing(void)
{
    /* x - y = 0.99 - (-0.99) = 1.98: overflows int16 if you're not careful. */
    lpf1_q15_t f;
    lpf1_q15_init(&f, q15_from_float(0.5f));
    lpf1_q15_update(&f, q15_from_float(-0.99f));
    q15_t y = lpf1_q15_update(&f, q15_from_float(0.99f));
    CHECK_NEAR(q15_to_float(y), 0.0, 1e-3);
}

static void test_lpf_q15_matches_float(void)
{
    lpf1_t ff;
    lpf1_init(&ff, 10.0f, 1000.0f);
    lpf1_q15_t fq;
    lpf1_q15_init(&fq, q15_from_float(ff.alpha));

    double worst = 0.0;
    for (int k = 0; k < 2000; k++) {
        float x = (k < 100) ? -0.5f : ((k < 1000) ? 0.8f : 0.1f);
        float yf = lpf1_update(&ff, x);
        float yq = q15_to_float(lpf1_q15_update(&fq, q15_from_float(x)));
        double err = fabs((double)yf - (double)yq);
        if (err > worst)
            worst = err;
    }
    printf("  worst |float - q15| = %.6f\n", worst);
    CHECK(worst < 1e-3);
}

int main(void)
{
    RUN(test_conversions);
    RUN(test_sat);
    RUN(test_add_sub_saturate);
    RUN(test_mul);
    RUN(test_lpf_q15_primes);
    RUN(test_lpf_q15_full_scale_swing);
    RUN(test_lpf_q15_matches_float);
    return UNIT_REPORT();
}
