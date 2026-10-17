# Lesson 6 — Arrays, structs and circular buffers

**You will learn:** how to store many values (arrays), how to group related
values (structs), how a struct maps onto a peripheral's registers, and the
circular buffer: the most-used data structure in embedded systems.

---

## 1. Arrays

An **array** is a row of values of the same type, stored back-to-back in
memory:

```c
int32_t samples[5] = {10, 20, 30, 40, 50};
//        index:      0   1   2   3   4

samples[0]        // 10: the first element is index 0
samples[4]        // 50: the last element is index (size - 1)
samples[2] = 99;  // change the middle one
```

Looping over an array:

```c
int64_t sum = 0;
for (uint32_t i = 0; i < 5; i++) {      // i = 0, 1, 2, 3, 4
    sum += samples[i];
}
```

`for (start; condition; step)` runs `start` once, then repeats the body
while `condition` is true, running `step` after each pass. `i++` means
"add 1 to i".

### ⚠️ C does not check array bounds

```c
samples[5] = 0;     // index 5 doesn't exist: writes past the end!
```

C won't stop you. You silently overwrite whatever variable lives next in
memory, and the program misbehaves somewhere else, much later. **Out-of-bounds
access is undefined behaviour** and one of the most common embedded bugs.
The project's tests use `-fsanitize=address`, which catches it on your PC.

### `sizeof`

`sizeof` gives a size in **bytes**:

```c
sizeof(int32_t)                    // 4
sizeof samples                     // 20 (5 × 4 bytes)
sizeof samples / sizeof samples[0] // 5: the number of elements
```

### Arrays and pointers

When you pass an array to a function, C passes the **address of its first
element**. The function receives a pointer and has no idea how long the
array is, so you always pass the length too:

```c
int64_t sum_of(const int32_t *values, uint32_t count);
sum_of(samples, 5);
```

## 2. Structs: grouping related data

A **struct** bundles several variables into one named thing:

```c
typedef struct {
    float kp, ki, kd;       // gains
    float integ;            // state
} pid_ctrl_t;

pid_ctrl_t motor_pid;       // one variable holding all four values
motor_pid.kp = 2.0f;        // the "." picks a member
```

`typedef` gives the struct a short name (`pid_ctrl_t`). By convention,
type names end in `_t`.

Usually you pass a **pointer** to a struct (Lesson 5: no copying, and the
function can modify it). With a pointer, use **`->`** instead of `.`:

```c
void pid_reset(pid_ctrl_t *p)
{
    p->integ = 0.0f;        // same as (*p).integ = 0.0f
}
pid_reset(&motor_pid);
```

This "struct + functions that take a pointer to it" pattern is how this
whole project is organised: `ringbuf_t` with `ringbuf_push(&rb, ...)`,
`pid_ctrl_t` with `pid_update(&pid, ...)`, and so on. It's C's version of an
object: you can have as many independent instances as you like.

## 3. Structs as register maps

A peripheral has many registers at consecutive addresses. GPIO port F
starts at `0x40021400`:

| Offset | Register |
|---|---|
| 0x00 | MODER (pin modes) |
| 0x04 | OTYPER |
| 0x08 | OSPEEDR |
| 0x0C | PUPDR (pull-ups) |
| 0x10 | IDR (input data) |
| 0x14 | ODR (output data) |
| 0x18 | BSRR (set/reset) |
| ... | ... |

Struct members are also laid out back-to-back in memory. So a struct of
`uint32_t`s, in the same order, **matches the hardware exactly**:

```c
typedef struct {
    volatile uint32_t MODER;    // +0x00
    volatile uint32_t OTYPER;   // +0x04
    volatile uint32_t OSPEEDR;  // +0x08
    volatile uint32_t PUPDR;    // +0x0C
    volatile uint32_t IDR;      // +0x10
    volatile uint32_t ODR;      // +0x14
    volatile uint32_t BSRR;     // +0x18
    /* ... */
} GPIO_TypeDef;

#define GPIOF ((GPIO_TypeDef *)0x40021400u)   // a pointer to port F's registers

GPIOF->ODR &= ~(1u << 9);   // reaches 0x40021400 + 0x14 = 0x40021414
```

`#define NAME text` is a **macro**: before compiling, the preprocessor
replaces every `GPIOF` with that text.

The same struct works for every port, since only the base address
differs. That's why `gpio_config(GPIOF, 9, ...)` can take the port as a
parameter. Open `firmware/include/stm32f407.h` to see all the register
structs. The `_Static_assert` lines there check at compile time that each
offset matches the manual.

## 4. The circular buffer (ring buffer)

**The problem:** the UART interrupt receives bytes whenever they arrive.
The main loop processes them when it gets a chance. You need a waiting
area between them: a **queue**, first in, first out (FIFO).

A plain array would fill up and you'd have to shift everything down after
each read, which is slow. Instead, treat the array as a **circle**: two
indexes chase each other around it.

```
        write here (head)
              ▼
 ┌───┬───┬───┬───┬───┬───┬───┬───┐
 │   │ B │ C │   │   │   │   │   │
 └───┴───┴───┴───┴───┴───┴───┴───┘
       ▲
   read here (tail)         after index 7 comes index 0 again
```

- **push** (producer, e.g. the ISR): store at `head`, then advance `head`.
- **pop** (consumer, e.g. main): read at `tail`, then advance `tail`.
- **empty** when `head == tail`.
- When an index passes the end, it **wraps** to 0.

Nothing is ever moved; only the two indexes advance. Every operation takes
the same tiny amount of time, no matter how full the buffer is. This
property is called **O(1)**, and it's exactly what you want in an ISR.

### Wrapping the index

```c
idx = (idx + 1) % SIZE;        // % = remainder: 7 → 8 % 8 = 0
```

`%` needs a division, which is slowish on a microcontroller. If `SIZE` is
a **power of two** (8, 16, 256...), there's a trick: `SIZE - 1` is a mask
of all ones (`8 - 1 = 0b0111`), so

```c
idx = (idx + 1) & (SIZE - 1);   // same result as % SIZE, but one cheap AND
```

### Telling "full" from "empty"

If the writer fills every slot, `head` catches up with `tail` and
`head == tail`, which looks exactly like "empty". There are two common
solutions:
1. Always leave one slot unused, so "full" means `head` is one behind `tail`.
2. Let `head` and `tail` **count up forever** (never wrapped), and only wrap
   them when you use them as an index (`buf[head & mask]`). Then
   `head - tail` = the number of items stored, and every slot is usable.

`lib/ringbuf.c` uses option 2. Question Q0.1 in Exercise 0 asks why
`head - tail` still works after the counters overflow past
`0xFFFFFFFF`. Lesson 2, section 6, has everything you need to answer it.

### Why it's safe with an interrupt

Remember the race condition from Lesson 5: trouble happens when **both**
sides read-modify-write the **same** variable. In a ring buffer, the
producer writes only `head` and the consumer writes only `tail`. Each one
only **reads** the other's index. With exactly one producer and one
consumer, they can't corrupt each other. That's why the UART ISR can push
while `main` pops, with no need to disable interrupts.

## 5. Application: the moving average (Exercise 2)

Sensor readings are noisy. A simple fix is to average the last N readings.
With a circular buffer of the last N samples, this is a **moving average**
(also called a boxcar filter).

The slow way re-adds all N samples every time. The fast way keeps a
**running sum**:

```
new sample arrives:
    sum = sum - (oldest sample, which is about to be overwritten)
    sum = sum + new sample
    store new sample in its slot
    average = sum / count
```

That's two operations instead of N, whatever N is.

**Think about overflow** (Lesson 3): if the samples are `int32_t` and N =
64, how big can the sum get? Does it still fit in 32 bits? Which type does
the sum need?

---

## Try it

```c
#include <stdio.h>
#include <stdint.h>

#define SIZE 4u   /* power of two */

int main(void)
{
    char buf[SIZE];
    uint32_t head = 0, tail = 0;

    const char *msg = "HELLO";
    for (int i = 0; msg[i] != '\0'; i++) {
        if (head - tail == SIZE) {            // full?
            printf("full, dropping '%c'\n", msg[i]);
            continue;
        }
        buf[head & (SIZE - 1u)] = msg[i];
        head++;
    }
    while (head != tail) {                    // pop everything
        printf("got '%c'\n", buf[tail & (SIZE - 1u)]);
        tail++;
    }
    return 0;
}
```

Change the code to start with `head = tail = 0xFFFFFFFEu`. Does it still
work? Why?

---

## Check yourself

1. An array is declared `uint16_t a[10];`. What are its first and last valid
   indexes? What is `sizeof a`?
2. When do you write `s.x` and when do you write `s->x`?
3. Why does `GPIOF->ODR` reach address `0x40021414`?
4. For a ring buffer of size 16, what is `(15 + 1) & 15`?
5. Why can the UART ISR push into the ring buffer while main pops, without
   disabling interrupts?

<details><summary>Answers</summary>

1. Indexes **0** and **9**. `sizeof a` = 10 × 2 = **20** bytes.
2. `s.x` when `s` **is** a struct; `s->x` when `s` is a **pointer** to a struct.
3. `GPIOF` is a pointer to address `0x40021400`, and `ODR` is the 6th
   `uint32_t` member at offset 5 × 4 = `0x14`.
4. 16 & 15 = `10000 & 01111` = **0**. It wraps to the start.
5. Each index has exactly one writer (the ISR writes `head`, main writes `tail`),
   so there's no read-modify-write on shared data.
</details>

> **Going deeper (optional):** there is one more subtlety. The producer
> must finish writing the byte **before** it publishes the new `head`, and
> the compiler (or CPU) must not reorder those two steps. `lib/ringbuf.c`
> guarantees the order with C11 atomics (`memory_order_release` and
> `memory_order_acquire`). You'll meet this properly in Phase 4 (RTOS).
