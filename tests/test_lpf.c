#include <math.h>

#include "lpf.h"
#include "unit.h"

static void test_alpha(void)
{
    lpf1_t f;
    lpf1_init(&f, 10.0f, 1000.0f);
    /* RC = 1/(2*pi*10) = 15.915 ms, dt = 1 ms -> alpha = 1/(15.915+1) */
    CHECK_NEAR(f.alpha, 0.0591174, 1e-5);
}

static void test_first_sample_primes(void)
{
    lpf1_t f;
    lpf1_init(&f, 10.0f, 1000.0f);
    CHECK_NEAR(lpf1_update(&f, 3000.0f), 3000.0, 1e-3);
    CHECK_NEAR(lpf1_update(&f, 3000.0f), 3000.0, 1e-3);
}

static void test_reinit_unprimes(void)
{
    lpf1_t f;
    lpf1_init(&f, 10.0f, 1000.0f);
    lpf1_update(&f, 5.0f);
    lpf1_init(&f, 10.0f, 1000.0f);
    CHECK_NEAR(lpf1_update(&f, -7.0f), -7.0, 1e-6);
}

static void test_step_response(void)
{
    lpf1_t f;
    lpf1_init(&f, 10.0f, 1000.0f);
    lpf1_update(&f, 0.0f);
    float y = 0.0f;
    for (int k = 1; k <= 20; k++) {
        y = lpf1_update(&f, 1.0f);
        CHECK_NEAR(y, 1.0 - pow(1.0 - 0.0591174, k), 1e-4);
    }
    /* After ~5 time constants the output is within 1% of the input. */
    for (int k = 0; k < 100; k++)
        y = lpf1_update(&f, 1.0f);
    CHECK(y > 0.99f && y <= 1.0f);
}

static void test_attenuates_high_frequency(void)
{
    /* 200 Hz sine through a 10 Hz filter at 1 kHz: theory says gain ~0.052. */
    lpf1_t f;
    lpf1_init(&f, 10.0f, 1000.0f);
    float peak = 0.0f;
    for (int k = 0; k < 2000; k++) {
        float x = sinf(2.0f * 3.14159265f * 200.0f * (float)k / 1000.0f);
        float y = lpf1_update(&f, x);
        if (k > 1000 && fabsf(y) > peak)
            peak = fabsf(y);
    }
    CHECK(peak > 0.03f && peak < 0.07f);
}

int main(void)
{
    RUN(test_alpha);
    RUN(test_first_sample_primes);
    RUN(test_reinit_unprimes);
    RUN(test_step_response);
    RUN(test_attenuates_high_frequency);
    return UNIT_REPORT();
}
