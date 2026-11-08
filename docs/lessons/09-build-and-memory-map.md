# Lesson 9 — From C code to a file on the chip

**You will learn:** what happens between saving `main.c` and the LED
blinking; how header and source files fit together; where each variable
ends up (flash, RAM, stack); and how to read the build's memory report, map
file and disassembly.

---

## 1. The build pipeline

```
 main.c ─┐                                                  ┌─► firmware.elf ──► debugger
 gpio.c ─┼─► preprocess ─► compile ─► gpio.o ─┐            │       │
 uart.c ─┘   (#include,    (C → ARM   uart.o ─┼─► LINK ────┤       └─ objcopy ─► firmware.bin ──► flash
              #define)     machine    main.o ─┘    ▲       │                     firmware.hex
                           code)                    │       └─► firmware.map (the report)
                                       linker script (.ld): where memory is
```

1. **Preprocessor:** handles lines starting with `#`. `#include "gpio.h"`
   pastes that file's text in; `#define LED0_PIN 9` replaces every `LED0_PIN`
   with `9`. It's pure text substitution, done before any real compiling.
2. **Compiler:** turns **each `.c` file separately** into an **object file**
   (`.o`) of ARM machine code. At this point, calls to functions in other files
   are left as "fill in the address later" notes.
3. **Linker:** combines all the `.o` files plus libraries (like the C
   library's `printf`), decides **where in memory** everything goes (using
   the linker script `firmware/linker/stm32f407zg.ld`), and fills in all the
   addresses. The output is an **ELF** file (`firmware.elf`), which contains
   the code plus debug information.
4. **objcopy:** strips the ELF down to the raw bytes that go into flash
   (`firmware.bin`).

`make firmware` runs all of this through CMake.

## 2. Header files and source files

Because each `.c` file is compiled alone, `main.c` needs to know what
functions `uart.c` offers, without seeing `uart.c` itself. That's the job
of a **header** (`.h`):

```c
/* uart.h: the PROMISE (declarations): "these functions exist" */
void uart1_init(uint32_t baud);
bool uart1_getc(char *c);

/* uart.c: the IMPLEMENTATION (definitions) */
#include "uart.h"
void uart1_init(uint32_t baud) { /* ... the actual code ... */ }

/* main.c: the USER */
#include "uart.h"
uart1_init(115200);        // the compiler checks the call against the promise
```

The header is the module's **public interface**. Anything not in the header
is private to its `.c` file.

### Header guards

```c
#ifndef UART_H
#define UART_H
/* ... */
#endif
```

This stops the header's contents from being pasted twice if it's included
from two places. Every header in this project has one.

### `static`: two different meanings

```c
static uint32_t s_ticks;          // at FILE level: private to this .c file

void f(void)
{
    static uint32_t calls = 0;    // INSIDE a function: keeps its value between
    calls++;                      // calls (it lives in RAM, not on the stack)
}
```

This project marks file-private variables with an `s_` prefix, and globals
shared between files with `g_`.

## 3. Where things live: sections

The linker sorts everything into **sections**:

| Section | Contains | Lives in | Example |
|---|---|---|---|
| `.text` | Machine code | Flash | every function |
| `.rodata` | Constants, strings | Flash | `const int table[] = {...};`, `"hello"` |
| `.data` | Global/static variables **with** an initial value | RAM (initial values stored in flash) | `uint32_t g_sysclk_hz = 16000000;` |
| `.bss` | Global/static variables **without** an initial value (or `= 0`) | RAM, zeroed at boot | `static uint32_t s_ticks;` |
| **stack** | Local variables, function return addresses | RAM (top end) | `int i;` inside a function |
| heap | `malloc()` memory | RAM | (avoid it, see below) |

### Why `.data` is special

RAM is erased at power-off, but `g_sysclk_hz` must start at 16,000,000. So
the initial value is stored in **flash**, and at every boot the startup code
**copies** it into RAM. It also **zeroes** `.bss`. You've seen this code:
it's steps 2 and 3 of `Reset_Handler` in
`firmware/startup/startup_stm32f407.c`, and the linker script provides the
addresses it needs (`_sidata`, `_sdata`, `_edata`, `_sbss`, `_ebss`).

### The stack

Every function call pushes its local variables and return address onto the
**stack**, a region at the top of RAM that grows **downwards**. When the
function returns, that space is freed instantly.

```
 0x20020000 ─ top of RAM ─ stack starts here, grows ↓
            │ main's locals        │
            │ uart1_write's locals │
            │      ↓ (free)        │
            │      ↑ (heap grows ↑)│
            │ .bss                 │
 0x20000000 │ .data                │
```

- A **big local array** (`uint8_t buf[4096];` inside a function) can
  overflow the small stack (4 KB in this project) and silently corrupt your
  other variables. Make big buffers `static` or global instead.
- Deep **recursion** (a function calling itself) is risky for the same reason.

### Why embedded code avoids `malloc`

`malloc` hands out memory at run time. On a device that must run for
years, this causes problems: it can fail when memory runs out, the
heap gets **fragmented** (enough free memory exists, but in pieces too
small to use), and how long it takes is unpredictable. The usual approach
is to allocate everything **statically**, at compile time, so the linker
tells you up front whether it fits.

## 4. Reading the memory report

`make firmware` prints:

```
Memory region         Used Size  Region Size  %age Used
           FLASH:       23112 B         1 MB      2.20%
             RAM:       13368 B       128 KB     10.20%

   text    data     bss     dec     hex  filename
  22584     472   12896   35952    8c70  firmware.elf
```

- **Flash used** = `text` + `data` (the code, constants, and the
  *initial values* of `.data`). Here that's 22,584 + 472 ≈ 23 KB.
- **RAM used** = `data` + `bss`. Here 472 + 12,896 ≈ 13 KB, and 12 KB of that
  is the space the linker script **reserves** for the stack (4 KB) and heap (8 KB).

Keep an eye on these numbers as the project grows.

## 5. The map file

`build/firmware.map` is the linker's full report: every function and
variable, its address, its size, and which `.o` file it came from. It's
long, so search it (in `less`, type `/name` and press Enter). A typical
entry:

```
 .bss.s_ticks   0x20000320    0x4   CMakeFiles/.../systick.c.obj
 └ section      └ address     └ size (bytes)  └ which source file
```

Address `0x2000....` means RAM (Lesson 5's memory map); `0x0800....` means
flash.

## 6. Other tools

**`arm-none-eabi-nm`**: list symbols (functions and variables).

```bash
arm-none-eabi-nm -S --size-sort build/firmware.elf | tail
```

The letter shows the section: `T` = text (code), `D` = data, `B` = bss,
`R` = read-only data. Lowercase means file-private (`static`).

**`arm-none-eabi-objdump -d`**: **disassemble**, showing the actual machine
instructions the compiler produced:

```bash
arm-none-eabi-objdump -d build/firmware.elf | less
```

You don't need to be fluent in ARM assembly. Just learn to recognise a few
things:
- `vadd.f32`, `vmul.f32` → hardware float maths (fast) ✓
- `bl __aeabi_dmul`, `bl __aeabi_dadd` → calls to software double-precision
  routines (slow): someone used a `double`!
- `ldr` / `str` → load from / store to memory (this is how `volatile`
  accesses appear)
- `b` / `bl` → branch (jump) / branch-and-link (function call)

**Godbolt** (<https://godbolt.org>, choose "ARM GCC") shows the
disassembly next to your C code as you type. It's the fastest way to see what
your C really becomes.

---

## Try it

After `make firmware`:

1. Search the map file for `SysTick_Handler`. Is it in flash or RAM?
2. Run `arm-none-eabi-nm -S --size-sort build/firmware.elf | tail -5`.
   What are the biggest functions? Do the names suggest where they come from?
3. In `main.c`, add `static uint8_t big[2048];` at file level and use it
   somewhere (e.g. `big[0] = 1;` in `main`). Rebuild. Which number in the
   memory report grew by 2048? Now give it an initial value
   (`= {1};`). What changes, and why?

---

## Check yourself

1. Which build step turns `gpio.c` into `gpio.o`? Which step decides the
   address of `gpio_config`?
2. Which section does each go in?
   (a) `const char msg[] = "hi";` (b) `uint32_t counter;` at file level
   (c) `uint32_t mode = 3;` at file level (d) `int i;` inside a function
3. Why are `.data` values stored in flash if the variables live in RAM?
4. Why is `uint8_t buffer[8192];` declared inside a function a bad idea here?
5. In a disassembly you see `bl __aeabi_dmul`. What does it tell you?

<details><summary>Answers</summary>

1. The **compiler** produces `gpio.o`; the **linker** assigns addresses.
2. (a) `.rodata` (flash); (b) `.bss`; (c) `.data`; (d) the **stack** (or a CPU register).
3. RAM loses its contents at power-off, so the initial values must be kept in flash and copied to RAM by the startup code at every boot.
4. It needs 8 KB of stack, but only 4 KB is reserved, so it overflows and corrupts other memory. Make it `static` or global.
5. The code does a **double**-precision multiply in software. Somewhere a `double` (perhaps a literal without `f`) slipped in.
</details>
