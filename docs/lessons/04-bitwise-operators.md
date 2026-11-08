# Lesson 4 — Bitwise operators

**You will learn:** the six operators that work on individual bits, and
the four idioms (set, clear, toggle, read) that make up most of every
hardware driver. Also: multi-bit fields, and why `1u` matters.

---

## 1. Why bit operations?

From Lesson 1: a register is 32 switches, and each bit controls something
different. Port F's output register controls **all 16 pins** of port F at
once. If you want to change PF9 only, you must **not disturb the other 15
pins**, which might be driving the buzzer (PF8) or the green LED (PF10).

Writing `register = 0x200` would set PF9 high **and force every other pin
low**. Bitwise operators let you change exactly the bits you mean.

## 2. The operators

They work on each bit position independently, comparing bit 0 with bit 0,
bit 1 with bit 1, and so on.

### AND `&`: 1 only if **both** bits are 1

```
  1100
& 1010
= 1000
```

Think of it as a **filter**: `x & mask` keeps the bits of `x` where `mask`
has 1s and forces everything else to 0.

### OR `|`: 1 if **either** bit is 1

```
  1100
| 1010
= 1110
```

Think of it as **adding bits**: `x | mask` forces the mask bits to 1 and
leaves the others as they were.

### XOR `^`: 1 if the bits are **different**

```
  1100
^ 1010
= 0110
```

Think of it as **flipping**: `x ^ mask` inverts the mask bits and leaves
the others alone.

### NOT `~`: inverts every bit

```
~ 0000 0000 0000 0000 0000 0010 0000 0000   (0x00000200)
= 1111 1111 1111 1111 1111 1101 1111 1111   (0xFFFFFDFF)
```

Use it to turn "a mask with one 1" into "a mask with one 0".

### Shift left `<<` and shift right `>>`

Shifting moves all bits left or right. Bits that fall off the end are lost,
and zeros come in at the other end.

```
0000 0001 << 3  =  0000 1000      (1 moved to position 3)
1011 0000 >> 4  =  0000 1011      (moved down by 4)
```

Shifting left by N multiplies by 2ᴺ, and shifting right divides by 2ᴺ.

> ⚠️ Don't confuse them with the **logical** operators `&&`, `||` and `!`.
> Those treat a whole value as true/false. `0x2 && 0x1` is `1` (both are
> non-zero, so "true"), but `0x2 & 0x1` is `0` (no common bits).

## 3. Building masks: `1u << n`

**"The number with only bit N set"** (Lesson 2) is written `1u << n`:

```c
1u << 0   == 0x001
1u << 3   == 0x008
1u << 9   == 0x200     // PF9
1u << 10  == 0x400     // PF10
(1u << 9) | (1u << 10) == 0x600   // both
```

This is much clearer than writing `0x200`: the reader can see it's about
bit 9.

### Why `1u` and not `1`?

`1` is a **signed** `int` (Lesson 3). `1 << 31` shifts a 1 into the
**sign bit** of a signed int, and the C standard calls that **undefined
behaviour**. `1u` is unsigned, so `1u << 31` is perfectly fine:
`0x80000000`. **Always write `1u` for bit masks.** The project's tests will
stop with an error if you don't.

Also undefined: shifting by **32 or more** (the full width of the type).
`1u << 32` doesn't give 0. It's UB. When the shift count is a variable, an
ARM CPU happens to produce 0, while a PC (x86) produces 1, because x86
only uses the low 5 bits of the shift count. So the same code behaves differently on two machines. Remember this
for `bit_mask(32)` in Exercise 1.

## 4. The four idioms (learn these by heart)

`reg` is a register or variable, and `n` is a bit number.

```c
reg |=  (1u << n);    // SET bit n to 1       (OR with mask)
reg &= ~(1u << n);    // CLEAR bit n to 0     (AND with inverted mask)
reg ^=  (1u << n);    // TOGGLE bit n         (XOR with mask)
(reg >> n) & 1u       // READ bit n → 0 or 1  (move it to bit 0, keep only bit 0)
```

`a |= b` is shorthand for `a = a | b`. The same goes for `&=`, `^=`, `<<=`
and `>>=`.

Let's check CLEAR on a byte with n = 2:

```
reg            = 1111 1111
1u << 2        = 0000 0100
~(1u << 2)     = 1111 1011
reg & that     = 1111 1011    ← only bit 2 changed
```

## 5. Multi-bit fields

Many registers use **groups** of bits for one setting. For example, the
"mode register" (`MODER`) gives every pin **2 bits**:

| Bits | Mode |
|---|---|
| 00 | input |
| 01 | output |
| 10 | alternate function (used by UART, timers...) |
| 11 | analog |

Pin *n* uses bits **2n and 2n+1**. So PF9's field is bits 18–19:

```
 bit:  ... 21 20 │ 19 18 │ 17 16 ...
           pin10 │ pin 9 │ pin 8
```

To set a field you need two steps, **clear then set**, because OR can only
turn bits on:

```c
reg &= ~(3u << 18);   // 1) clear both bits of the field (3 = binary 11)
reg |=  (1u << 18);   // 2) write 01 = output
```

To read a field: shift it down to bit 0, then mask off its width:

```c
uint32_t mode = (reg >> 18) & 3u;    // 0, 1, 2 or 3
```

In general, a mask of **w** ones is `(1u << w) - 1`:

```
w = 2:  (1u << 2) - 1  =  100 - 1  =  011  = 3
w = 4:  (1u << 4) - 1  = 10000 - 1 = 01111 = 0xF
```

(But think about what happens when w = 32... see section 3.)

## 6. Read-modify-write

The "clear then set" steps are a **read-modify-write**: the CPU **reads**
the register, **modifies** the value, and **writes** it back. That's three
separate steps. In Lesson 5 you'll see why this can go wrong when an
interrupt sneaks in between them, and how the STM32's `BSRR` register
avoids the problem for GPIO pins.

## 7. Bits in this project

Open `firmware/drivers/gpio.c` and `firmware/drivers/gpio.h`. Now you can
read them:

```c
port->BSRR = high ? (1u << pin) : (1u << (pin + 16u));
```

`BSRR` is a special register: writing a 1 to bit *n* sets pin *n*
high, and writing a 1 to bit *n + 16* sets pin *n* low.

```c
static void set_field(volatile uint32_t *reg, uint32_t pos, uint32_t width, uint32_t value)
{
    uint32_t mask = ((1u << width) - 1u) << pos;
    *reg = (*reg & ~mask) | ((value << pos) & mask);
}
```

This is section 5 in one line. (`*reg` and `volatile` come in Lesson 5.)

---

## Try it

```c
#include <stdio.h>
#include <stdint.h>

static void print_bin(uint32_t x)          // prints the low 16 bits
{
    for (int i = 15; i >= 0; i--) {
        putchar(((x >> i) & 1u) ? '1' : '0');
        if (i % 4 == 0) putchar(' ');
    }
    putchar('\n');
}

int main(void)
{
    uint32_t reg = 0x00F0;
    print_bin(reg);
    reg |= (1u << 9);   print_bin(reg);   // set bit 9
    reg &= ~(1u << 5);  print_bin(reg);   // clear bit 5
    reg ^= (1u << 0);   print_bin(reg);   // toggle bit 0
    printf("bit 9 is %u\n", (reg >> 9) & 1u);
    return 0;
}
```

Predict each printed line before you run it.

---

## Check yourself

1. What is `0xF0 & 0x3C`? What is `0xF0 | 0x3C`? What is `0xF0 ^ 0x3C`?
2. Write the C statement that clears bit 10 of `reg` without touching other bits.
3. Why `1u << 31` and not `1 << 31`?
4. PE4's mode field: which bit positions? Write the two lines that set it to input (`00`).
5. Is `(reg & (1u << 9))` a valid way to test bit 9 in an `if`? What value
   does it give when bit 9 is set?

<details><summary>Answers</summary>

1. `1111 0000 & 0011 1100` = `0011 0000` = **0x30**; OR = `1111 1100` = **0xFC**; XOR = `1100 1100` = **0xCC**.
2. `reg &= ~(1u << 10);`
3. `1` is a signed int, and shifting into its sign bit is undefined behaviour. `1u` is unsigned, so the result is well-defined.
4. Bits **8 and 9** (2 × 4 = 8). `reg &= ~(3u << 8);` For input (`00`) there's nothing to OR in, so the clear alone is enough. Writing `reg |= (0u << 8);` would do nothing.
5. Yes. It gives **0x200** (512) when the bit is set and 0 otherwise. Any non-zero value counts as "true" in an `if`. To get exactly 0 or 1, use `(reg >> 9) & 1u`.
</details>
