# Lesson 1 — What is a microcontroller?

**You will learn:** what is inside the chip on your board, how a program
controls real hardware, and how a program starts running.

---

## 1. A whole computer on one chip

Look at the big black square in the middle of your board, the one labelled
**STM32F407ZGT6**. It's a **microcontroller (MCU)**: a complete small
computer on a single chip. It contains:

| Part | What it is | Everyday comparison |
|---|---|---|
| **CPU** (ARM Cortex-M4) | Carries out instructions, up to 168 million per second | The brain |
| **Flash memory** (1 MB) | Holds your program. **Kept** when power is off. | A book of instructions |
| **RAM** (192 KB) | Holds variables while the program runs. **Erased** when power is off. | A whiteboard for scratch notes |
| **Peripherals** | Built-in hardware helpers: pins (GPIO), timers, serial ports (UART), analog inputs (ADC), and more | Hands, ears and mouth |

```
            ┌──────────────────── STM32F407 ─────────────────────┐
            │                                                     │
            │   ┌──────┐    ┌──────────┐    ┌───────────┐         │
            │   │ CPU  │◄──►│  Flash   │    │    RAM    │         │
            │   │ M4F  │    │ (program)│    │(variables)│         │
            │   └──┬───┘    └──────────┘    └───────────┘         │
            │      │  bus (the CPU reads/writes addresses)        │
            │   ┌──┴─────┬──────────┬──────────┬──────────┐       │
            │   │ GPIO   │ Timers   │  UART    │   ADC    │ ...   │
            │   └──┬─────┴────┬─────┴────┬─────┴────┬─────┘       │
            └──────┼──────────┼──────────┼──────────┼─────────────┘
                   │          │          │          │   ← pins
                 LEDs,      motors     USB-serial  sensors
                 buttons    (PWM)      to your PC
```

**Compared with your laptop:** a laptop runs an operating system (Windows,
macOS, Linux) that sits between your program and the hardware. A
microcontroller usually has **nothing in between**. Your program is the only
thing running and it controls the hardware **directly**. That is what
"embedded" and "bare metal" mean.

Microcontrollers are everywhere: washing machines, car engines, drones,
keyboards, thermostats, medical pumps. A modern car has more than 50 of
them.

## 2. A tour of your board

The chip is the brain; the board around it supplies power and connects
the chip's pins to useful things. On your ALIENTEK Explorer:

- **Two LEDs** (DS0 red, DS1 green): your first outputs
- **Four buttons** (KEY0, KEY1, KEY2, WK_UP): your first inputs
- **A buzzer**
- **USB_232 port**: a USB-to-serial chip (CH340) so the MCU can send text to your PC
- **A 20-pin JTAG/SWD header**: where a debugger (ST-LINK) plugs in
- Lots of extras for later: EEPROM, SPI flash, light sensor, LCD
  connector, CAN, RS485, Ethernet...

`docs/BOARD.md` lists exactly which chip pin each one connects to.

## 3. Pins, ports and the LED

The chip has 144 legs. Most of them are **general-purpose input/output
(GPIO)** pins. They are grouped into **ports** named A, B, C ... G, with
16 pins each, numbered 0–15. A pin's name combines the two: **PF9** means
**P**ort **F**, pin **9**.

The red LED is wired to **PF9**. A pin used as an output can be:

- **high**: the pin outputs 3.3 V
- **low**: the pin outputs 0 V (connected to ground)

How the LED is wired decides which one turns it on. On your board, the
LED's other leg goes to 3.3 V:

```
 3.3 V ──►|── resistor ── PF9
        LED
```

Current only flows (and the LED only lights) when PF9 is **low**, because
then there's a voltage difference across the LED. This is called
**active low**, and it's very common. So on this board:

| PF9 | LED |
|---|---|
| low (0) | **on** |
| high (1) | off |

## 4. Registers: how code touches hardware

This is the most important idea in the whole course.

Each peripheral is controlled by **registers**. A register is a small
32-bit storage slot at a **fixed address**, which the CPU can read and
write just like a variable in RAM. The difference is that the bits of a
register are **wired to real hardware**.

For example, port F has an "output data register" at address
`0x40021414`. (Lesson 2 explains the `0x` notation.) Its 32 bits behave like
a row of switches, and **bit 9 is connected to pin PF9**:

```
 bit:   31 ...  10   9   8  ...  1   0
       ┌───┬───┬───┬───┬───┬───┬───┬───┐
       │   │   │   │ 1 │   │   │   │   │   bit 9 = 1  →  PF9 high (3.3 V)
       └───┴───┴───┴─┬─┴───┴───┴───┴───┘
                     └──────────────► pin PF9 → LED
```

> **Controlling hardware = writing the right number to the right address.**

Every driver you will ever write, for UART, timers, motors or sensors,
is this one idea repeated with different registers. The
**reference manual** (RM0090, about 1,700 pages) is the dictionary that
tells you which register lives at which address and what each bit does.
You don't read it cover to cover; you look things up in it, like a
dictionary.

## 5. How a program gets onto the chip and starts

```
 your .c files ──compile──► firmware.elf / .bin ──flash──► chip's flash memory
   (on PC)                   (machine code)       (ST-LINK or USB serial)
```

When the chip powers on or you press RESET:

1. The CPU reads the **first two numbers** stored in flash. The first says
   where the **stack** (scratch memory for function calls) is in RAM. The
   second is the address of the **first instruction** to run.
2. That first code (`Reset_Handler`, in
   `firmware/startup/startup_stm32f407.c`) prepares RAM for your
   variables.
3. It calls your **`main()`**.

## 6. The shape of every embedded program

On a PC, a program runs, finishes and returns to the operating system.
On a microcontroller there's no operating system to return to, and the
device must keep working as long as it has power. So `main()` **never
returns**:

```c
int main(void)
{
    setup_everything();          // runs once: clocks, pins, UART...

    for (;;) {                   // "forever" loop: runs until power-off
        read_inputs();           // buttons, sensors
        decide_what_to_do();     // your logic / control algorithm
        drive_outputs();         // LEDs, motors
    }
}
```

`for (;;)` is a loop with no stop condition: an infinite loop. Open
`firmware/src/main.c` in this repo and you'll find exactly this
structure.

---

## Check yourself

1. Which part of the chip keeps your program when the power is off: flash or RAM?
2. What does the name **PE4** mean?
3. On this board, to turn the red LED **on**, should PF9 be high or low? Why?
4. In one sentence: what is a register?
5. Why does `main()` on a microcontroller end in an infinite loop?

<details><summary>Answers</summary>

1. **Flash.** RAM is erased at power-off.
2. **Port E, pin 4.** On this board it's the KEY0 button.
3. **Low.** The LED's other end is at 3.3 V, so current flows only when PF9 is at 0 V (active low).
4. A small storage slot at a fixed address whose bits are wired to hardware. Writing it controls the hardware; reading it reports the hardware's state.
5. There's no operating system to return to, and the device must keep doing its job as long as it's powered.
</details>
