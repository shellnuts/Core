/*
 * unit.h - A deliberately tiny test framework (no dependencies).
 * Graduate to Unity or CppUTest once you outgrow it.
 */
#ifndef UNIT_H
#define UNIT_H

#include <math.h>
#include <stdio.h>

static int unit_failures;
static int unit_checks;

#define CHECK(cond)                                                        \
    do {                                                                   \
        unit_checks++;                                                     \
        if (!(cond)) {                                                     \
            unit_failures++;                                               \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
        }                                                                  \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                              \
    do {                                                                   \
        double a_ = (a), b_ = (b);                                         \
        unit_checks++;                                                     \
        if (fabs(a_ - b_) > (tol)) {                                       \
            unit_failures++;                                               \
            printf("  FAIL %s:%d: %s = %g, expected %g +/- %g\n",          \
                   __FILE__, __LINE__, #a, a_, b_, (double)(tol));         \
        }                                                                  \
    } while (0)

#define RUN(test)                                                          \
    do {                                                                   \
        printf("%s\n", #test);                                             \
        test();                                                            \
    } while (0)

#define UNIT_REPORT()                                                      \
    (printf("%d checks, %d failures\n", unit_checks, unit_failures),       \
     unit_failures == 0 ? 0 : 1)

#endif
