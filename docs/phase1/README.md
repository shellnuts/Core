# Phase 1 — Embedded C

**Goal:** write C that is correct on a microcontroller: no undefined behaviour,
no hidden overflow, no wasted cycles, and every line testable on your PC.

> **New to C or electronics?** Read the [lessons](../lessons/README.md)
> first. Each exercise below names the lesson it needs.

**How this phase works:** each exercise has
- a **header** in `lib/` that defines the API and the rules (read it first, carefully),
- a **stub** `.c` file with `TODO`s for you to fill in,
- a **test** in `tests/` that tells you when you're done.

```bash
make phase1                         # scoreboard: which exercises pass
make -C tests test_bits && ./tests/test_bits   # details for one exercise
```

The tests run with AddressSanitizer and UndefinedBehaviorSanitizer, so a
`1 << 31` or an out-of-bounds write fails loudly with the exact line number
instead of "working" on your PC and breaking on the board.

Work in order; each exercise builds on the one before. Commit after each one.
Write your answers to the ✍️ questions in `docs/phase1/NOTES.md`.
Hints are folded. Try for at least 20 minutes before opening them.

---

## Exercise 0 — Read before you write (1 evening)

📖 **Read first:** Lessons 1–3 and 5

Read these files from the starter and make sure you understand every line:
`lib/ringbuf.c`, `firmware/drivers/systick.c`, `firmware/drivers/gpio.c`.

✍️ **Q0.1** In `ringbuf.c`, `head` and `tail` are never reset. They just keep
counting up and eventually wrap from `0xFFFFFFFF` to `0`. Why does
`head - tail` still give the right count after one has wrapped and the
other hasn't? Work through an example: `head = 0x00000002`, `tail = 0xFFFFFFFE`.

✍️ **Q0.2** Why is `s_ticks` in `systick.c` declared `volatile`? Describe
exactly what could go wrong in `delay_ms()` without it at `-O2`.

✍️ **Q0.3** `gpio_write()` uses `BSRR`, but `gpio_toggle()` uses `ODR`. Why is
the first one safe to call from an interrupt and the second one not?

✍️ **Q0.4** Integer promotion. What does each of these evaluate to, and why?
```c
uint8_t a = 200, b = 100;
uint8_t  c = a + b;         // ?
int      d = a + b;         // ?
uint16_t x = 0xFFFF;
bool     e = (x + 1) == 0;  // ?  (hint: what type is x + 1?)
int      f = -1;
bool     g = f < 1u;        // ?  (compile with -Wall -Wextra: what does gcc say?)
```
Check your answers by compiling a small test program.

---

## Exercise 1 — Bit manipulation · `lib/bits.h`

📖 **Read first:** Lesson 4

**Concepts:** masks, shifts, read-modify-write, the `1 << 31` trap,
why shifting by ≥ the type width is undefined.

Every line of a register driver is one of these operations. After this
exercise, `gpio.c`'s `set_field()` should look obvious.

<details><summary>Hint 1</summary>

`1` is an `int` (signed, 32 bits). `1 << 31` overflows into the sign bit,
which is undefined behaviour. Use `1u`.
</details>
<details><summary>Hint 2 (bit_mask(32))</summary>

`(1u << 32) - 1` is undefined because the shift count equals the type width. Handle
`width == 32` as a special case, or build the mask another way:
`0xFFFFFFFFu >> (32 - width)`.
</details>

**Then on the board:** rewrite `gpio_config()` in `firmware/drivers/gpio.c`
to use `field_set()` (`lib/` is already on the firmware's include path),
rebuild, and check that LED0 still blinks.

---

## Exercise 2 — Moving average · `lib/moving_avg.c`

📖 **Read first:** Lesson 6

**Concepts:** circular buffers with `%` vs. power-of-two masks, running
sums, choosing integer widths so arithmetic can't overflow, integer division
semantics.

<details><summary>Hint 1</summary>

When the window is full, the oldest sample is exactly the one at `idx`,
which you're about to overwrite. Subtract it from the sum first.
</details>
<details><summary>Hint 2</summary>

64 × `INT32_MAX` needs 38 bits. The sum must be `int64_t`, and the addition
must happen *in* 64 bits: `m->sum += sample` is fine because `sample` is
converted to the type of `sum` first.
</details>

✍️ **Q2.1** Your `idx = (idx + 1) % n` compiles to a division instruction
(`udiv`, 2–12 cycles on the M4). How does `ringbuf.c` avoid that, and what does
it cost in flexibility?

---

## Exercise 3 — First-order low-pass filter · `lib/lpf.c`

📖 **Read first:** Lesson 7 (Parts A and C)

**Concepts:** discretising an RC circuit, `float` vs `double` on a
single-precision FPU, filter time constants, priming filter state.

<details><summary>Hint</summary>

`3.14159265` (no `f`) is a `double`. On the board, any expression it
touches is computed in software double precision: about 20× slower.
The firmware is built with `-Wdouble-promotion`, which flags it.
</details>

✍️ **Q3.1** What is alpha for a 10 Hz cutoff at 1 kHz sampling?
✍️ **Q3.2** After how many samples does a step input reach 63% of its
final value? How does that relate to RC?
✍️ **Q3.3** Compare it with the moving average: which one uses less memory? Which
has a cleaner frequency response? Which is better for removing a single
spike?

---

## Exercise 4 — Debouncer state machine · `lib/debounce.c`

📖 **Read first:** Lesson 8

**Concepts:** explicit state machines (`enum` + `switch`), events vs.
levels, wrap-safe time comparison, designing code that's testable without
hardware (time is a *parameter*, not a call to `millis()`).

<details><summary>Hint 1</summary>

Write the `switch` with all four `case`s first, each with only a `break;`.
Then fill in one state at a time. Re-read the timing rule in the header.
</details>
<details><summary>Hint 2 (wrap-around)</summary>

`now >= since + stable` fails when `since + stable` wraps past zero but
`now` hasn't wrapped yet. `now - since >= stable` is
always right: unsigned subtraction gives the elapsed time modulo 2³².
That's the same reason Q0.1 works.
</details>

**Then on the board** (this is the payoff):
1. Add `lib/debounce.c` to `CMakeLists.txt`.
2. In `main.c`, keep one `debounce_t` per key and update them every
   millisecond (in the main loop, using `millis()`).
3. Make each KEY0 **press** toggle LED1, and print `KEY0 press` / `KEY0
   release` over the UART.
4. Compare it with the raw `board_key_pressed()`: count raw edges per press and
   print them. How many bounces do the Explorer's buttons produce?

---

## Exercise 5 — Q15 fixed point · `lib/q15.c`

📖 **Read first:** Lesson 7 (Part B)

**Concepts:** Q-format numbers, saturation vs. wrap-around, rounding,
intermediate widths, the one product (`-1 × -1`) that doesn't fit, and
converting floats to integers safely.

<details><summary>Hint 1 (q15_from_float)</summary>

Converting a float that's out of range to `int` is undefined behaviour
(`(int)1e30f` can be anything). Compare and saturate while it's still a float.
`lroundf()` from `<math.h>` rounds to nearest.
</details>
<details><summary>Hint 2 (lpf1_q15_update)</summary>

Do the whole update in `int32_t`: `diff = (int32_t)x - y` (range ±65535),
`step = (alpha * diff + (1 << 14)) >> 15`, then saturate `y + step`.
</details>

✍️ **Q5.1** The test prints the worst-case error between the float and Q15
filters. Explain where the error comes from. Hint: what happens to
`alpha * diff` when `diff` is tiny? Look up "limit cycle" or "deadband" in
fixed-point IIR filters.

✍️ **Q5.2** Stretch: write `pid_q15` (or Q16.16 with `int32_t`). Why is Q15
awkward for gains like `kp = 2.0`?

---

## Exercise 6 — Where did everything go? (1 evening)

📖 **Read first:** Lesson 9

```bash
make firmware
less build/firmware.map
arm-none-eabi-nm --size-sort -S build/firmware.elf | tail -20
arm-none-eabi-objdump -d build/firmware.elf | less
```

✍️ **Q6.1** Find `s_rx_storage` in the map file. Which section and address
is it in, and why that section?
✍️ **Q6.2** What are the five largest symbols? (You'll see what
`-u _printf_float` costs.)
✍️ **Q6.3** Find `g_sysclk_hz`. It has an initial value. Where is that value
stored in flash, and which code copies it to RAM at boot?
✍️ **Q6.4** Disassemble `pid_update`. Do you see `vmul.f32`/`vadd.f32` (FPU)
or calls to `__aeabi_dmul` (software double)? Add a `double` somewhere,
rebuild, and look again.

---

## You're done with Phase 1 when
- [ ] `make phase1` shows 5/5
- [ ] all ✍️ questions are answered in `NOTES.md`
- [ ] KEY0 toggles LED1 cleanly through your debouncer on the real board
- [ ] you can explain, without notes: `volatile`, why `1u`, wrap-safe time
      comparison, and why the sum in the moving average is 64-bit

Then move on to Phase 2 (Cortex-M core and bare metal) in `docs/ROADMAP.md`.
