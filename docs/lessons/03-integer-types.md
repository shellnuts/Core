# Lesson 3 — Integer types in C

**You will learn:** what a variable is, how to pick the right integer type,
how negative numbers are stored, what happens when numbers overflow, and
the hidden conversions C performs behind your back.

---

## 1. Variables and types

A **variable** is a named piece of memory that holds a value. Its **type**
tells the compiler how many bits it uses and how to interpret them.

```c
int count = 0;        // create a variable named count, start it at 0
count = count + 5;    // now it's 5
```

Every statement ends with `;`. Text after `//` is a **comment**: notes for
humans that the compiler ignores.

## 2. Fixed-width types: always use these in embedded code

C's classic types (`char`, `short`, `int`, `long`) **don't have a fixed
size**: `int` is 16 bits on some microcontrollers and 32 on others. When
you're writing to a 32-bit register, "probably 32 bits" isn't good enough.

So embedded code uses the types from `<stdint.h>`. The name tells you
everything:

```
 uint32_t
 │   │  └─ "_t" = it's a type
 │   └──── 32 bits
 └──────── u = unsigned (0 and up); without u = signed (can be negative)
```

| Type | Bits | Range |
|---|---|---|
| `uint8_t` | 8 | 0 to 255 |
| `int8_t` | 8 | −128 to 127 |
| `uint16_t` | 16 | 0 to 65,535 |
| `int16_t` | 16 | −32,768 to 32,767 |
| `uint32_t` | 32 | 0 to 4,294,967,295 |
| `int32_t` | 32 | −2,147,483,648 to 2,147,483,647 |
| `uint64_t` / `int64_t` | 64 | very large |

Also useful: `bool` (from `<stdbool.h>`), with values `true` and `false`.

**How to choose:**
- Register values and bit patterns → `uint32_t`
- Counts and indexes that can't be negative → unsigned (`uint32_t`, or `size_t` for sizes)
- Values that can be negative (temperatures, errors, speeds) → signed (`int32_t`)
- Big arrays of small values (e.g. a 1,000-sample buffer) → the smallest
  type that fits, to save RAM

On the STM32, **32-bit operations are the fastest**. Using `uint8_t` for a
loop counter doesn't speed anything up; the CPU works in 32 bits anyway.

## 3. How negative numbers are stored: two's complement

Bits are just 0s and 1s. There's no minus sign, so how do you store −1?

Every modern CPU uses **two's complement**: in a signed type, the top bit
counts as **negative**. For 8 bits:

```
position:   7     6    5    4    3    2    1    0
worth:   -128    64   32   16    8    4    2    1
```

```
0000 0001 =                         1
0111 1111 =  64+32+16+8+4+2+1   = 127   (largest)
1000 0000 = -128                = -128  (smallest)
1111 1111 = -128+127            = -1
1111 1110 = -128+126            = -2
```

So **the same bits mean different numbers depending on the type**.
`1111 1111` is 255 as a `uint8_t` and −1 as an `int8_t`. The bits
don't know; only the type does.

To negate a number in two's complement: flip all the bits, then add 1.
`5 = 0000 0101` → flip → `1111 1010` → +1 → `1111 1011` = −5.

## 4. Overflow: unsigned wraps, signed is "undefined behaviour"

Lesson 2 showed that **unsigned** numbers wrap around (255 + 1 = 0 in a
byte). C guarantees this.

**Signed** overflow is different. If an `int32_t` holding 2,147,483,647
has 1 added to it, the C standard says the result is **undefined
behaviour (UB)**.

**Undefined behaviour** means the language makes no promise about what
happens. It might wrap to a negative number. Or the compiler, which is
allowed to assume UB never happens, might optimise your code into
something you never wrote. The program can work in testing and fail in
the field. **Treat UB as a bug, always.**

```c
int32_t  s = 2147483647;  s = s + 1;   // UNDEFINED: a bug
uint32_t u = 4294967295u; u = u + 1;   // fine: u == 0, guaranteed
```

The project's tests compile with `-fsanitize=undefined`, a checker that
stops the program and prints the line number when UB happens.

## 5. Integer literals and the `u` suffix

A number written in your code is a **literal**. Its type matters:

| Literal | Type |
|---|---|
| `5` | `int` (signed, 32 bits on ARM and PCs) |
| `5u` | `unsigned int` (32 bits) |
| `0x200` | `int` (it fits in an int) |
| `5.0` | `double` (64-bit decimal: see Lesson 7) |
| `5.0f` | `float` (32-bit decimal) |

Why should you care? Because `1` is a **signed** `int`. Lesson 4 shows how
`1 << 31` goes wrong for exactly this reason, and how `1u << 31` fixes it.

## 6. Hidden conversions: integer promotion

This rule surprises everyone:

> **Before doing arithmetic, C converts any type smaller than `int`
> (like `uint8_t` and `uint16_t`) to `int`.**

This is called **integer promotion**. Example:

```c
uint8_t a = 200, b = 100;
int sum = a + b;     // a and b become int, 200 + 100 = 300 (no wrap!)
```

The arithmetic happens in 32-bit `int`, so the result is 300 even though
both inputs were bytes. Wrap-around only happens if you then **store**
the result into a smaller type: storing 300 into a `uint8_t` keeps only the
low 8 bits.

So the type of the **calculation** and the type of the **variable you store
into** are two separate things. Keep them apart in your head.

## 7. Mixing signed and unsigned: a classic trap

When an operation combines an `int` and an `unsigned int`, C converts the
`int` to **unsigned**. A negative number then becomes a huge positive one:

```c
int x = -1;
unsigned int y = 1;
if (x < y) { ... }   // x becomes 4294967295 → the comparison is FALSE!
```

The compiler warns about this with `-Wall -Wextra` ("comparison of integers
of different signedness"). **Read your warnings.**

Rule of thumb: don't mix signed and unsigned in the same expression. If you
must, convert one of them explicitly with a **cast**.

## 8. Casts: explicit conversions

A **cast** converts a value to another type on purpose:

```c
int32_t big = 1000;
uint8_t small = (uint8_t)big;   // keeps only the low 8 bits: 1000 % 256 = 232

int32_t a = 70000, b = 70000;
int64_t product = (int64_t)a * b;   // cast BEFORE multiplying: done in 64 bits
```

The second example matters. `a * b` = 4.9 billion doesn't fit in 32 bits.
Casting the result afterwards is too late; the overflow has already
happened. Cast an **input** so the multiplication itself is done in 64
bits. You'll need this in Exercises 2 and 5.

## 9. Integer division truncates

```c
7 / 2    == 3     // not 3.5: the fraction is thrown away
-7 / 2   == -3    // truncates toward zero
7 % 2    == 1     // % gives the remainder
```

---

## Try it

Predict each output **before** running it:

```c
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    int8_t s = -1;
    uint8_t u = (uint8_t)s;
    printf("-1 stored as uint8_t: %u\n", u);

    uint8_t a = 200, b = 100;
    int as_int = a + b;
    uint8_t as_byte = a + b;
    printf("as int: %d, as byte: %u\n", as_int, as_byte);

    int32_t x = 70000;
    int64_t wrong = x * x;            // what goes wrong here?
    int64_t right = (int64_t)x * x;
    printf("wrong: %lld  right: %lld\n", (long long)wrong, (long long)right);

    printf("7/2 = %d, -7/2 = %d, 7%%2 = %d\n", 7 / 2, -7 / 2, 7 % 2);
    return 0;
}
```

Then compile it with `gcc -Wall -Wextra -fsanitize=undefined` and run it
again. Which line does the sanitizer complain about?

---

## Check yourself

1. Which type would you use for: (a) a 32-bit register value, (b) a
   temperature in °C, (c) an array of 2,000 ADC samples between 0 and 4095?
2. What are the bits of −1 in an `int8_t`? What value do those same bits
   have in a `uint8_t`?
3. Which one is a bug: unsigned overflow or signed overflow? Why?
4. `uint16_t x = 65535;`. What type does C use to compute `x + 1`, and what is the result?
5. Why is `(int64_t)(a * b)` wrong when `a` and `b` are large `int32_t` values?

<details><summary>Answers</summary>

1. (a) `uint32_t`; (b) `int32_t` or `int16_t` (it can be negative);
   (c) `uint16_t` (it fits in 16 bits and uses half the RAM of `uint32_t`).
2. `1111 1111`; as `uint8_t` that's **255**.
3. **Signed overflow** is undefined behaviour. Unsigned overflow wraps, and that's guaranteed.
4. `x` is promoted to `int`, so the result is **65536** (an `int`). No wrap, unless you store it back into a `uint16_t`.
5. `a * b` is computed in 32 bits first and has already overflowed (UB) before the cast happens. Use `(int64_t)a * b`.
</details>
