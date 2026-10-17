# Lesson 7 — Decimals on a chip: float, fixed point and filters

**You will learn:** how computers store numbers like 0.25, why `float` and
`double` behave very differently on your STM32, how fixed point does
decimals with plain integers, and what a filter actually does to a noisy
signal.

---

## Part A — Floating point

### 1. `float` and `double`

Integers can't hold 3.7 or 0.001. For that, C has **floating-point**
types:

| Type | Bits | Precision | On your STM32F407 |
|---|---|---|---|
| `float` | 32 | about 7 significant digits | **Fast**: done in hardware by the FPU |
| `double` | 64 | about 15–16 digits | **Slow**: done in software, roughly 10–50× slower |

The Cortex-M4F has a **floating-point unit (FPU)**: hardware that does
`float` maths in 1–14 clock cycles. But it only handles **single precision**
(32-bit). Every `double` operation becomes a call to a library function
that does the maths step by step with integers.

**Rule for this chip: use `float` everywhere, never `double`.**

### 2. The `f` suffix trap

```c
float y = x * 0.5;       // ✗ 0.5 is a DOUBLE: x is converted to double,
                         //   multiplied in software, converted back
float y = x * 0.5f;      // ✓ 0.5f is a float: one hardware instruction
```

A decimal literal without `f` is a `double`, and it "infects" the whole
expression. The same goes for maths functions: `sin()` is double, `sinf()` is
float. The firmware compiles with `-Wdouble-promotion`, which warns you when
this happens.

### 3. How floats store numbers (just the idea)

A float stores a number like scientific notation in binary:
**sign × mantissa × 2^exponent**, using 1 sign bit, 8 exponent bits and 23
mantissa bits. Consequences:

- **Precision is relative.** Near 1.0, the steps between representable
  floats are tiny (about 0.0000001). Near 16,000,000, the step is 1 or 2.
  `16777216.0f + 1.0f == 16777216.0f`. The +1 is lost!
- **Most decimals aren't exact.** 0.1 can't be stored exactly in binary,
  just as 1/3 can't be written exactly in decimal. `0.1f` is really
  stored as 0.100000001490..., and adding `0.1f` ten times gives
  1.00000012, not exactly 1.0.
- **Never compare floats with `==`.** Check that they're close enough:
  `fabsf(a - b) < 1e-6f`.
- Integer counters must not be stored in floats. A float that counts
  milliseconds stops increasing after about 4.6 hours.

### 4. Float ↔ integer conversion

```c
float f = 3.99f;
int32_t i = (int32_t)f;     // 3: conversion truncates toward zero
int32_t r = lroundf(f);     // 4: round to nearest (from <math.h>)
```

⚠️ Converting a float that's **too large for the integer type** (like
`(int16_t)1e9f`) is **undefined behaviour**. Clamp the value first, while
it's still a float. Exercise 5 tests exactly this.

## Part B — Fixed point

### 5. Decimals with integers

Before FPUs were common, and still today on cheap chips without one (like
Cortex-M0 or M3), people used **fixed point**: you agree on a **scale
factor** and store `value × scale` in an integer.

Example: store money as cents. €12.34 → integer `1234` (scale 100).
Additions just work: 1234 + 66 = 1300 → €13.00.

In embedded code, the scale is a **power of two**, so scaling is a cheap
shift instead of a division.

### 6. The Q15 format

**Q15** stores numbers between −1 and +1 in an `int16_t`, with scale
2¹⁵ = 32768:

```
real value = raw / 32768          raw = round(real × 32768)

 0.5    → 16384  = 0x4000
 0.25   →  8192  = 0x2000
−0.5    → −16384
−1.0    → −32768 = 0x8000   (the most negative int16)
+1.0    → 32768?  doesn't fit in int16_t (max 32767): the largest is 0.99997
```

"15" means 15 bits after the binary point. The 16th bit is the sign.

### 7. Q15 arithmetic

**Addition:** add the raw values (scales match). But 0.75 + 0.75 = 1.5,
which is out of range. In 16 bits it would **wrap** to a negative number, which
is a disaster for a control signal. Instead, **saturate**: clamp to the
largest or smallest value.

```
raw: 24576 + 24576 = 49152 → doesn't fit → clamp to 32767 (≈ +1.0)
```

So: do the sum in a **wider type** (`int32_t`, where it can't overflow),
then clamp back into the range [−32768, 32767].

**Multiplication:** multiplying two scaled numbers multiplies the scales too:

```
(a × 32768) × (b × 32768) = (a × b) × 32768²     ← scale is now 2³⁰ (a "Q30")
```

so shift right by 15 to get back to Q15:

```
0.5 × 0.5:   16384 × 16384 = 268,435,456   (needs 32 bits: use int32_t!)
             268,435,456 >> 15 = 8192  = 0.25 ✓
```

**Rounding:** `>> 15` simply drops the fractional part, so it always rounds
down. To round to nearest, add half of the step first:
`(product + (1 << 14)) >> 15`.

**The one special case:** −1 × −1 = +1, which Q15 can't represent.
Saturate it to 32767.

### 8. Why learn fixed point if you have an FPU?

- Many microcontrollers don't have one.
- DSP libraries (CMSIS-DSP), audio and sensor data often come in Q15/Q31.
- It shows where precision is lost: every shift throws away bits, and
  you'll see the effect in Exercise 5.

## Part C — Filters

### 9. Signals and noise

Read a sensor 1000 times per second and plot the values. You'll see the
**real signal** (slow changes) plus **noise** (fast, random jitter from
electrical interference, ADC quantisation, vibration):

```
true value:   ────────────────╱‾‾‾‾‾‾‾‾‾‾‾
measured:     ─╮╭╮─╭─╮╭──╭╮─╱╲‾╮╭‾‾╮╭‾╮‾‾
```

A **filter** keeps the slow part and removes the fast part. Because it lets
**low** frequencies **pass**, it's called a **low-pass filter**.

Frequency, measured in hertz (Hz), means "how many times per second
something wiggles". Mains hum is 50 Hz; a motor speed change might happen
over 0.5 s (a couple of Hz).

### 10. The moving average (Lesson 6)

Average the last N samples. It's simple and good at removing random
noise, but it needs N samples of memory, and every sample in the window
counts equally, even old ones.

### 11. The exponential (first-order) low-pass filter

A cheaper and very popular alternative needs **one** stored value:

```c
y = y + alpha * (x - y);
```

- `x` is the new measurement, and `y` is the filtered output.
- `x - y` is how far the measurement is from our current estimate.
- We move a **fraction `alpha`** of the way towards the measurement.

`alpha` is between 0 and 1:
- **alpha close to 1**: follow the input quickly (little smoothing).
- **alpha close to 0**: move slowly (lots of smoothing, but more lag).

Step response (input jumps from 0 to 1, alpha = 0.5):

```
sample:   0     1     2      3       4
y:        0   0.5   0.75  0.875  0.9375  → approaches 1
```

Each step closes a fixed fraction of the remaining gap, so it approaches the
target along an **exponential curve**. That's the same curve as charging a
capacitor through a resistor.

### 12. From RC circuit to alpha

In electronics, a resistor R and capacitor C make a low-pass filter. It
has:
- a **time constant τ = RC** (seconds): after one τ, a step input has reached
  **63%** of its final value. After 5τ, it's above 99%.
- a **cutoff frequency f_c = 1 / (2π·RC)**: wiggles slower than f_c pass;
  faster ones are attenuated, more and more strongly as frequency rises.

Our code version runs once every **dt** seconds (dt = 1 / sample rate).
The digital equivalent of the RC circuit uses:

```
RC    = 1 / (2π · f_c)
alpha = dt / (RC + dt)
```

Example: you want a 10 Hz cutoff and you sample at 1000 Hz (dt = 0.001 s):

```
RC    = 1 / (2 × 3.14159 × 10) = 0.0159 s
alpha = 0.001 / (0.0159 + 0.001) ≈ 0.059
```

So each sample moves the output about 6% of the way towards the input.
Exercise 3 asks you to implement exactly this.

### 13. Priming the filter

If `y` starts at 0 and the first measurement is 3000, the output crawls up
from 0 for many samples, a fake transient. **Priming** fixes it: the
**first** sample sets `y = x` directly, and normal filtering starts with
the second one.

---

## Try it

```c
#include <stdio.h>
#include <math.h>

int main(void)
{
    /* Float precision */
    float big = 16777216.0f;
    printf("16777216 + 1 = %.1f\n", (double)(big + 1.0f));
    float sum = 0.0f;
    for (int i = 0; i < 10; i++)
        sum += 0.1f;
    printf("0.1 added 10 times = %.9f, equal to 1.0? %s\n",
           (double)sum, (sum == 1.0f) ? "yes" : "no");

    /* Q15 by hand */
    short a = 16384, b = 16384;              /* 0.5 and 0.5 */
    int product = (int)a * b;
    printf("0.5*0.5 in Q15 = %d (= %f)\n", product >> 15, (product >> 15) / 32768.0);

    /* Low-pass filter on a step */
    float y = 0.0f, alpha = 0.2f;
    for (int k = 0; k < 15; k++) {
        y = y + alpha * (1.0f - y);
        printf("k=%2d  y=%.3f\n", k, (double)y);
    }
    return 0;
}
```

(The `(double)` casts are only there because `printf` expects doubles for
`%f`. That's fine on a PC.)

How many steps does the filter take to pass 0.63? Compare with
`1 / alpha`.

---

## Check yourself

1. Why is `x * 0.5` slow on the STM32F407 when `x` is a `float`?
2. What is 0.25 in Q15 (raw value)? What real value is raw `−8192`?
3. Why must a Q15 multiply be done in `int32_t`?
4. In `y = y + alpha * (x - y)`, what does a smaller alpha do?
5. Compute alpha for a 5 Hz cutoff at 1000 Hz sampling.

<details><summary>Answers</summary>

1. `0.5` is a `double` literal, so the multiplication is done in double precision, in software. Write `0.5f`.
2. 0.25 × 32768 = **8192** (`0x2000`). −8192 / 32768 = **−0.25**.
3. The product of two Q15 values needs up to 31 bits (it's a Q30), so it overflows 16 bits.
4. More smoothing and more lag: the output reacts more slowly.
5. RC = 1/(2π × 5) = 0.0318 s; alpha = 0.001 / (0.0318 + 0.001) ≈ **0.0305**.
</details>
