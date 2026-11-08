# Lesson 2 — Binary and hexadecimal

**You will learn:** how the chip stores numbers, how to read binary and
hex, how to find "the number for bit N", and what happens when a number
gets too big (wrap-around).

---

## 1. Bits

Inside the chip, everything is stored as tiny switches that are either
**off (0)** or **on (1)**. One switch is called a **bit**. Every number,
letter and instruction is a pattern of bits.

## 2. Binary: counting with only 0 and 1

In everyday **decimal** (base 10), each digit position is worth **10×**
the one to its right:

```
  3   5   7      =  3×100 + 5×10 + 7×1  =  357
 100  10   1
```

In **binary** (base 2), each position is worth **2×** the one to its right:

```
position:   7    6    5    4    3    2    1    0
worth:    128   64   32   16    8    4    2    1
```

To read a binary number, **add up the worth of every position holding a 1**:

```
0000 0101  =  4 + 1               =   5
0000 1010  =  8 + 2               =  10
0110 0100  =  64 + 32 + 4         = 100
1000 0001  =  128 + 1             = 129
1111 1111  =  128+64+32+16+8+4+2+1 = 255
```

(The space after every 4 digits is only there to make it readable.)

> **Bit positions are numbered from 0, starting at the RIGHT.**
> "Bit 0" is the rightmost bit. "Bit 9" is the 10th from the right.
> The worth of bit N is 2ᴺ: bit 0 = 1, bit 1 = 2, bit 3 = 8, bit 9 = 512.

### Decimal → binary

Go from the biggest worth downwards. Take each worth if it fits, write 1,
and subtract it; otherwise write 0. Example: **37**.

```
128? no →0   64? no →0   32? yes →1 (37-32=5)   16? no →0
  8? no →0    4? yes →1 (5-4=1)   2? no →0   1? yes →1 (1-1=0)
37 = 0010 0101
```

## 3. Sizes

| Name | Bits | Unsigned range |
|---|---|---|
| bit | 1 | 0 – 1 |
| **byte** | 8 | 0 – 255 |
| half-word | 16 | 0 – 65,535 |
| **word** | 32 | 0 – 4,294,967,295 |

With N bits you can store 2ᴺ different values, from 0 to 2ᴺ − 1.
Your STM32 is a **32-bit** microcontroller: its registers and its natural
number size are 32 bits.

## 4. Hexadecimal: shorthand for binary

A 32-bit value in binary is unreadable:

```
0100 0000 0000 0010 0001 0100 0001 0100
```

**Hexadecimal** (base 16) fixes that. **Every group of 4 bits becomes one
hex digit.** 4 bits count from 0 to 15, so hex needs 16 symbols: 0–9,
then **A–F** for 10–15.

| Binary | Hex | Dec | | Binary | Hex | Dec |
|---|---|---|---|---|---|---|
| 0000 | 0 | 0 | | 1000 | 8 | 8 |
| 0001 | 1 | 1 | | 1001 | 9 | 9 |
| 0010 | 2 | 2 | | 1010 | **A** | 10 |
| 0011 | 3 | 3 | | 1011 | **B** | 11 |
| 0100 | 4 | 4 | | 1100 | **C** | 12 |
| 0101 | 5 | 5 | | 1101 | **D** | 13 |
| 0110 | 6 | 6 | | 1110 | **E** | 14 |
| 0111 | 7 | 7 | | 1111 | **F** | 15 |

**Learn this table by heart.** You'll use it every day in embedded work.

In C, hex numbers start with **`0x`**, so `0x10` (sixteen) is never
confused with `10` (ten).

### Binary → hex

Split into groups of 4 from the right, then swap each group for its digit:

```
0100 0000 0000 0010 0001 0100 0001 0100
  4    0    0    2    1    4    1    4     →  0x40021414
```

That's the port F output register address from Lesson 1.

### Hex → binary

Swap each digit for its 4 bits: `0xA5` → `1010 0101`.

### Hex → decimal

Each hex position is worth 16× the previous one (1, 16, 256, 4096...):
`0x2A` = 2×16 + 10 = **42**. In practice you rarely need decimal; engineers
think in hex and binary when dealing with registers.

## 5. "The number with only bit N set"

This comes up constantly: to control pin N, you need the number that has
a 1 in position N and 0 everywhere else. Its value is 2ᴺ.

```
bit 0  →  0000 0000 0001  = 0x001  =   1
bit 3  →  0000 0000 1000  = 0x008  =   8
bit 4  →  0000 0001 0000  = 0x010  =  16
bit 9  →  0010 0000 0000  = 0x200  = 512
```

**Shortcut:** every 4 bits is one hex digit, so work out which hex digit
the bit falls in (N ÷ 4) and which bit inside that digit (N mod 4 → 1, 2, 4
or 8). For bit 9: 9 ÷ 4 = 2 remainder 1, so it's the third hex digit from
the right, with value 2 → `0x200`.

Several bits at once: OR them together (Lesson 4), or just add them, since
they don't overlap. Bits 9 and 10 → `0x200 + 0x400` = `0x600`.

## 6. Wrap-around: when a number runs out of bits

A byte holds 0–255. What is 255 + 1?

```
   1111 1111   (255)
 + 0000 0001     (1)
 = 1 0000 0000       ← that needs 9 bits, but a byte only has 8
```

The 9th bit has nowhere to go and is **lost**. What's left is
`0000 0000` = **0**. The number **wraps around**, just like a car odometer:

```
 99998 → 99999 → 00000 → 00001
```

Subtraction wraps the other way: in a byte, **0 − 1 = 255**.

In general, an N-bit unsigned value computes everything **modulo 2ᴺ**: you
keep only the remainder after dividing by 2ᴺ (256 for a byte). For
example, 250 + 10 = 260, and 260 − 256 = **4**.

In C, wrap-around of **unsigned** numbers is **guaranteed by the language**.
It isn't a bug or an error, and good embedded code deliberately relies on
it. You'll meet it again with the ring buffer (Lesson 6) and with
timekeeping (Lesson 8): a millisecond counter in 32 bits wraps back to 0
after 49.7 days, and correct code keeps working when that happens.

---

## Try it

```c
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint8_t a = 255;
    a = a + 1;
    printf("255 + 1 in a byte = %u\n", a);

    uint8_t b = 0;
    b = b - 1;
    printf("0 - 1 in a byte   = %u\n", b);

    printf("0x200 in decimal  = %d\n", 0x200);
    printf("42 in hex         = 0x%X\n", 42);
    return 0;
}
```

(`uint8_t` means "unsigned 8-bit number". Lesson 3 covers it properly.)

---

## Check yourself

1. Convert binary `0001 0000` to decimal.
2. Convert `0x2A` to binary.
3. The green LED is on **PF10**. Write the hex number with only bit 10 set.
4. Which hex number has bits 0, 1, 2 and 3 all set?
5. In an 8-bit unsigned number, what is `250 + 10`? What is `3 − 5`?
6. How many different values can 16 bits hold?

<details><summary>Answers</summary>

1. **16** (only bit 4 is set: 2⁴ = 16).
2. `0x2A` → 2 = `0010`, A = `1010` → **`0010 1010`** (= 42).
3. **`0x400`** (2¹⁰ = 1024; bit 10 = third hex digit, bit 2 inside it → 4).
4. **`0xF`** (`1111`).
5. 250 + 10 = 260 → 260 − 256 = **4**. 3 − 5 = −2 → −2 + 256 = **254**.
6. 2¹⁶ = **65,536** (0 to 65,535).
</details>
