# Lessons — the knowledge behind the exercises

Written for **absolute beginners**. Read them in order; each one builds on
the ones before. Every lesson ends with **Check yourself** questions (answers
are folded underneath; try first!) and most have small **Try it**
experiments you can run on your PC.

| # | Lesson | You need it for |
|---|---|---|
| 1 | [What is a microcontroller?](01-microcontroller.md) | The big picture |
| 2 | [Binary and hexadecimal](02-binary-and-hex.md) | Everything |
| 3 | [Integer types in C](03-integer-types.md) | Phase 1, Exercise 0 |
| 4 | [Bitwise operators](04-bitwise-operators.md) | Exercise 1 (`bits.h`) |
| 5 | [Memory, pointers, registers and `volatile`](05-memory-pointers-volatile.md) | Exercise 0, all drivers |
| 6 | [Arrays, structs and circular buffers](06-arrays-structs-buffers.md) | Exercise 2 (`moving_avg`) |
| 7 | [Decimals on a chip: float, fixed point and filters](07-float-fixed-point-filters.md) | Exercises 3 and 5 (`lpf`, `q15`) |
| 8 | [Time and state machines](08-time-and-state-machines.md) | Exercise 4 (`debounce`) |
| 9 | [From C code to a file on the chip](09-build-and-memory-map.md) | Exercise 6 (map file) |

After each lesson, do the matching exercise in
[`docs/phase1/README.md`](../phase1/README.md).

## Running small C programs on your PC

Many lessons say **Try it**. You need a C compiler on your computer:

- **Linux:** `sudo apt install gcc`
- **macOS:** run `xcode-select --install`
- **Windows:** install WSL (Ubuntu) and use the Linux command, or use
  MSYS2 and run `pacman -S gcc`
- **No install:** paste the code into <https://www.onlinegdb.com> (choose
  language "C") or <https://godbolt.org> (which also shows the generated
  machine code)

Save the code as `try.c`, then:

```bash
gcc -Wall -Wextra -o try try.c && ./try
```

`-Wall -Wextra` turns on warnings. Always use them. A warning is the
compiler telling you that you probably made a mistake.
