# Embedded control on the STM32F407 Explorer

A learning workspace for embedded C, Cortex-M internals, peripheral drivers,
RTOS, sensors and actuators, and control algorithms. It targets the
**ALIENTEK Explorer (探索者) STM32F407ZGT6** board.

- **[docs/ROADMAP.md](docs/ROADMAP.md)**: phase-by-phase plan with exercises and checklists
- **[docs/BOARD.md](docs/BOARD.md)**: board pin map, documents to download, how to flash

## What's here

```
firmware/
  startup/   vector table + reset handler, written in C (no assembly, no ST code)
  linker/    linker script: where code, data, stack and heap go
  include/   hand-written STM32F407 register map (struct per peripheral)
  drivers/   clock (168 MHz PLL), SysTick, GPIO, USART1 (IRQ + ring buffer)
  board/     Explorer LEDs, keys, buzzer
  src/       main.c (command shell + PID demo), syscalls.c (printf -> UART)
lib/         portable code, built for the MCU AND unit-tested on your PC
  ringbuf    lock-free SPSC FIFO (ISR -> main loop)
  pid        discrete PID: Tustin integral, filtered D on measurement, anti-windup
  plant.h    simulated first-order plant for practising without hardware
tests/       host unit tests (`make test`)
tools/       plot_telemetry.py: capture CSV from the board and plot it
```

Everything is bare metal, with no HAL. You see every register write. Once you
understand that layer, ST's HAL and LL drivers and STM32CubeMX are easy to
pick up (roadmap phase 3).

## Setup

**Toolchain** (any OS):
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`), CMake ≥ 3.20, make
  - Ubuntu/Debian: `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi cmake make openocd`
  - macOS: `brew install --cask gcc-arm-embedded && brew install cmake openocd`
  - Windows: use WSL, or install the Arm toolchain, CMake and make (MSYS2)
- To flash: an ST-LINK + OpenOCD, **or** `stm32flash` over the board's USB_232 port (see [docs/BOARD.md](docs/BOARD.md))
- Python 3 with `pip install pyserial matplotlib` for the serial monitor and plots
- Recommended editor: VS Code with the *Cortex-Debug* extension

## Quick start

```bash
make test        # run the PC-side unit tests: no board needed
make phase1      # phase 1 exercise scoreboard (docs/phase1/README.md)
make firmware    # -> build/firmware.elf / .bin / .hex
make flash       # ST-LINK on the JTAG/SWD header
#   or: make flash-isp SERIAL=/dev/ttyUSB0  (BOOT0=1, press RESET first)
make monitor     # 115200 8N1 on the USB_232 port
```

On the board:
- **LED0** blinks at 1 Hz, so you know the clock and SysTick are working.
- **LED1** lights while **KEY0** is held. **WK_UP** sounds the buzzer.
- In the serial terminal, press `h` for help, `i` for clock info, and `s` to run a PID step response against a simulated plant.

`make plot` runs the simulation on the board and plots setpoint, output and
control effort. That's your first closed-loop experiment. Next, swap the simulated plant for a
real one (roadmap phase 6).

## Conventions
- C11, `-Wall -Wextra`, no warnings allowed.
- Hardware access lives in `firmware/drivers` and `firmware/board`. Algorithms live in `lib/` and must compile and be tested on the host.
- Commit after each exercise. Your git log becomes your learning journal.
