# Lesson 5 — Memory, pointers, registers and `volatile`

**You will learn:** what memory addresses are, what pointers are and how to
use them, how C code reaches a hardware register, what an interrupt is, and
why `volatile` exists.

This is the lesson where C turns into **embedded** C. Take your time.

---

## 1. Memory is a long row of numbered boxes

Picture memory as a street with billions of houses. Each house holds
**one byte** and has a **house number**, called its **address**.

```
address:   0x20000000  0x20000001  0x20000002  0x20000003  ...
contents:     0x2A        0x00        0x00        0x00
```

A `uint32_t` takes 4 bytes, so it occupies 4 neighbouring addresses. Its
address is the address of its first byte.

On the STM32, **everything** has an address in one big 32-bit map:

| Address range | What lives there |
|---|---|
| `0x0800 0000` … | Flash: your program |
| `0x2000 0000` … | RAM: your variables |
| `0x4000 0000` … | **Peripheral registers**: GPIO, UART, timers... |
| `0xE000 0000` … | CPU core registers: SysTick, interrupt controller |

The CPU uses the **same** instructions to read a variable in RAM and to
read a GPIO register. The only difference is the address.

## 2. Pointers: variables that hold addresses

A **pointer** is a variable whose value is an address.

```c
uint32_t speed = 42;        // a normal variable, somewhere in RAM
uint32_t *p = &speed;       // p holds the ADDRESS of speed
```

- `uint32_t *p` declares "p is a pointer to a `uint32_t`". The `*` in a
  declaration means "pointer to".
- `&speed` means "the address of speed".

Using the pointer:

```c
*p = 100;                   // "the thing p points at" = 100 → speed is now 100
uint32_t x = *p;            // read through the pointer → x = 100
```

The `*` **in an expression** means "go to that address": it
**dereferences** the pointer.

```
   p                       speed
 ┌────────────┐          ┌──────┐
 │ 0x20000010 │ ───────► │ 100  │   (lives at address 0x20000010)
 └────────────┘          └──────┘
```

### Why do pointers exist?

1. **To let a function change the caller's variable.** C passes arguments
   by **copy**, so a function that receives a copy can't change the
   original:

   ```c
   void reset(uint32_t *value) { *value = 0; }
   reset(&speed);              // speed is now 0
   ```

   This is how `uart1_getc(char *c)` in this project hands a received
   character back to you, while its return value says whether there was one.

2. **To avoid copying big things.** Pass the address of a 1 KB struct
   (8 bytes on a PC, 4 on the STM32) instead of copying 1 KB.

3. **To reach hardware registers**, as explained next.

`NULL` is a pointer that points at nothing. Dereferencing it is a bug (UB).

## 3. Reaching a hardware register

From Lesson 1, the port F output register (`ODR`) is at address
`0x40021414`. In C:

```c
volatile uint32_t *gpiof_odr = (volatile uint32_t *)0x40021414u;

*gpiof_odr &= ~(1u << 9);    // clear bit 9 → PF9 low → red LED ON
```

Reading the first line from right to left:
- `0x40021414u` is just a number.
- `(volatile uint32_t *)` is a **cast** that says "treat this number as the
  address of a 32-bit hardware register".
- `*gpiof_odr` means "the register at that address".

That's all a register access is. You can even write it in one line, with no
variable:

```c
*(volatile uint32_t *)0x40021414u |= (1u << 9);   // red LED off
```

In practice nobody writes raw addresses everywhere. Lesson 6 shows how a
**struct** gives every register in a peripheral a name. That's how
`GPIOF->ODR` works in `firmware/include/stm32f407.h`.

## 4. Interrupts

Your `main()` loop runs forever, but hardware events happen at
unpredictable times: a byte arrives on the UART, a timer expires, a button
is pressed. Checking for them constantly in the loop (**polling**) is wasteful
and can miss events.

An **interrupt** is the hardware tapping the CPU on the shoulder:

```
main loop:  ──run──run──run──┐               ┌──run──run──►
                             │  (paused)     │
interrupt!                   ▼               │
handler:                     └─ISR runs──────┘
```

1. The event happens, for example the UART receives a byte.
2. The CPU **pauses** your code wherever it is, even mid-statement.
3. It runs a special function called an **interrupt service routine
   (ISR)**, also known as a handler.
4. It **resumes** your code exactly where it stopped. Your code never
   notices the pause.

In this project, `SysTick_Handler()` (in `firmware/drivers/systick.c`)
runs every 1 ms because a timer interrupt fires, and `USART1_IRQHandler()`
(in `uart.c`) runs whenever a byte arrives.

**ISR rules:** keep them short and fast (microseconds). Never wait or loop
inside them. Hand data to the main loop through variables or buffers.

## 5. The problem `volatile` solves

The compiler **optimises** your code. One trick it uses is to keep a
variable's value in a CPU register instead of re-reading memory, if it
can "see" that nothing changes it. Look at this:

```c
static uint32_t ticks;                 // incremented by SysTick_Handler

void wait_one_tick(void)
{
    uint32_t start = ticks;
    while (ticks == start) {           // wait for the ISR to change ticks
    }
}
```

Inside `wait_one_tick`, **nothing** writes `ticks`. The compiler doesn't
know an interrupt will change it, because interrupts are invisible in normal
C code. So at `-O2` it may read `ticks` **once**, conclude the loop
condition can never change, and produce an **infinite loop**. The code works
with optimisation off and hangs with it on. That's a classic beginner
mystery.

**`volatile` tells the compiler: "this value can change behind your back,
so read it from memory every single time, and perform every write I
ask for."**

```c
static volatile uint32_t ticks;        // ✓ now the loop re-reads it every time
```

Use `volatile` for:
1. **Hardware registers.** Their values change by themselves (an input pin,
   a "data received" flag), and writing to them has effects. A write must
   never be skipped or merged with another one.
2. **Variables shared between an ISR and the main code.**

That's why every register in `stm32f407.h` is declared `__IO` (which is
`#define`d to `volatile`).

> `volatile` does **not** make code thread-safe or atomic. It only stops
> the compiler from skipping or caching accesses. For safe sharing, read
> on.

## 6. Race conditions: when an interrupt arrives at the wrong moment

Consider a counter that both `main` and an ISR increment:

```c
volatile uint32_t events;
// in main:  events = events + 1;
// in ISR:   events = events + 1;
```

`events = events + 1` looks like one step, but the CPU does **three**:

```
1. LOAD  r0 ← events        (read)
2. ADD   r0 ← r0 + 1        (modify)
3. STORE events ← r0        (write)
```

Now suppose the interrupt fires between steps 1 and 3 of `main`'s update:

```
events = 5
main:  LOAD r0 ← 5
                         ISR: LOAD 5, ADD → 6, STORE events = 6
main:  ADD r0 → 6
main:  STORE events = 6        ← the ISR's increment is LOST (should be 7)
```

This is a **race condition**. It happens rarely, maybe once in a million
runs, which makes it a nightmare to debug. The pattern that causes it is
**read-modify-write** (Lesson 4, section 6) on data that an ISR also
writes.

Ways to avoid it:
- **Don't share writes.** Make only one side write each variable. The ring
  buffer in Lesson 6 works this way: the ISR writes only `head`, and main
  writes only `tail`.
- **Use hardware that does it in one step.** Some registers are designed so
  that a single write does the whole job, with no read needed first.
- **Disable interrupts briefly** around the read-modify-write (`__disable_irq()`
  / `__enable_irq()`). It's simple, but it delays every interrupt, so keep the
  protected section tiny.

Exercise 0's question Q0.3 asks you to apply this section to `gpio_write()`
(which uses `BSRR`) and `gpio_toggle()` (which uses `ODR`). Re-read Lesson 4,
section 7, for what `BSRR` does.

## 7. `const`: the other qualifier

`const` means "this code won't change it":

```c
const uint32_t baud = 115200;           // can't be modified
void print(const char *text);           // promises not to modify the text
```

Global `const` data stays in **flash** instead of using precious RAM
(Lesson 9). Use `const` for lookup tables, strings and configuration.

---

## Try it

Pointers on your PC:

```c
#include <stdio.h>
#include <stdint.h>

static void double_it(uint32_t *v) { *v = *v * 2; }

int main(void)
{
    uint32_t a = 21;
    uint32_t *p = &a;
    printf("a = %u, lives at %p\n", a, (void *)&a);
    printf("p holds %p, *p = %u\n", (void *)p, *p);
    *p = 50;
    printf("after *p = 50: a = %u\n", a);
    double_it(&a);
    printf("after double_it: a = %u\n", a);
    return 0;
}
```

`volatile` in action: paste this into <https://godbolt.org>, choose
"ARM GCC" with option `-O2`, and compare the machine code for the loop with
and without `volatile`. Without it, you'll see the loop branch to itself
without ever reloading `flag`.

```c
#include <stdint.h>
uint32_t flag;   // try: volatile uint32_t flag;
void wait(void) { while (flag == 0) { } }
```

---

## Check yourself

1. What do `&x` and `*p` mean?
2. Write a function `void add(int32_t *total, int32_t amount)` that adds
   `amount` to the caller's variable.
3. What two kinds of things should be `volatile`?
4. Is `count++` safe when both `main` and an ISR execute it? Why or why not?
5. In the address map, is `0x40021414` RAM, flash or a peripheral?

<details><summary>Answers</summary>

1. `&x` is the address of `x`; `*p` is the value at the address stored in `p` (dereference).
2. `void add(int32_t *total, int32_t amount) { *total += amount; }`, called as `add(&sum, 5);`
3. Hardware registers, and variables shared between an ISR and other code.
4. **No.** `count++` is a read-modify-write (load, add, store). An interrupt between the load and the store makes one increment disappear.
5. A **peripheral** register (the `0x4000 0000` range). It's GPIOF's ODR.
</details>
