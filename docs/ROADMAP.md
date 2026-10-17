# Learning roadmap — embedded control on the STM32F407 Explorer

Each phase lists **concepts**, **exercises** (do them in this repo) and a
**done when** check. Tick the boxes as you go. Times assume roughly 8–10 hours a week.

---

## Phase 1 — Embedded C (2–4 weeks)

Concepts: bit manipulation, `volatile`, `const`, `static`, fixed-width
types, integer promotion, pointers to registers, structs and unions, memory
sections (`.text/.data/.bss`/stack/heap), fixed-point compared with float.

**Guided exercises: [docs/phase1/README.md](phase1/README.md)** (run `make phase1` for the scoreboard)

- [ ] Ex 0: read the starter code; questions on wrap-around, `volatile`, integer promotion
- [ ] Ex 1: bit and bit-field helpers (`lib/bits.h`)
- [ ] Ex 2: moving-average filter (`lib/moving_avg.c`)
- [ ] Ex 3: first-order low-pass filter (`lib/lpf.c`)
- [ ] Ex 4: debouncer state machine (`lib/debounce.c`), then KEY0 → LED1 on the board
- [ ] Ex 5: Q15 fixed point + a fixed-point low-pass compared against float (`lib/q15.c`)
- [ ] Ex 6: read the map file and disassembly

**Done when:** `make phase1` shows 5/5, the questions in `docs/phase1/NOTES.md` are answered, and the debounced KEY0 works on the board.

## Phase 2 — Cortex-M4 core and bare metal (3–4 weeks)

Concepts: boot sequence, vector table, linker script, clock tree and PLL,
flash wait states, NVIC priorities and preemption, SysTick, exceptions,
HardFault analysis, FPU lazy stacking.

- [ ] Flash the starter (`make flash` or `make flash-isp`) and talk to it with `make monitor`.
- [ ] Step through `Reset_Handler` in the debugger. Watch `.data` get copied.
- [ ] Change the PLL to 84 MHz. What has to change elsewhere? (BRR, SysTick, flash latency)
- [ ] Output SYSCLK/4 on the MCO2 pin (PC9) and measure it with a logic analyzer.
- [ ] Use EXTI interrupts for the keys instead of polling.
- [ ] Write a `HardFault_Handler` that prints PC, LR, CFSR, HFSR and BFAR. Trigger it on purpose (`*(int*)0x1 = 0;`).
- [ ] Measure interrupt latency: toggle a GPIO in an ISR and scope it against the trigger.

**Done when:** you can draw the boot sequence from memory and debug a HardFault to the offending line.

## Phase 3 — Peripheral drivers (4–6 weeks)

Write each driver with registers first. Then compare it with ST's LL and HAL drivers (STM32CubeF4).

- [ ] **Timers:** PWM on LED0 (TIM14_CH1, PF9) with breathing brightness.
- [ ] **Timers:** generate a 1 kHz control-loop interrupt from TIM6. Toggle a pin and measure the jitter.
- [ ] **Input capture:** decode the IR remote (PA8). Measure the PWM duty cycle of an external signal.
- [ ] **Encoder mode:** TIM2/TIM3/TIM4 with a quadrature encoder (motor phase).
- [ ] **ADC:** read the light sensor (PF7). Then sample it timer-triggered at 1 kHz with DMA in circular mode.
- [ ] **DAC + ADC loop:** wire PA4 to an ADC pin. Output a sine wave and read it back.
- [ ] **UART:** TX by interrupt, then by DMA. Make the command shell parse `kp 1.5`.
- [ ] **I2C:** driver for the 24C02. Store and load PID gains from EEPROM.
- [ ] **SPI + DMA:** read the W25Q128 JEDEC ID, then log telemetry to flash.

**Done when:** you have a `firmware/drivers/` layer, and the application code never touches a register directly.

## Phase 4 — RTOS (3–4 weeks)

Add FreeRTOS (the kernel source, or through CubeMX) to this project.

Concepts: tasks, priorities, the preemptive scheduler, PendSV/SVC context
switching, queues, semaphores, mutexes, priority inversion, task notifications,
`FromISR` APIs, stack sizing, static allocation, tickless idle.

- [ ] Port the starter to tasks: `control` (highest, 1 kHz from a TIM6 ISR notification), `sensors`, `telemetry` (queue → UART DMA), `shell`, `ui` (LEDs and keys).
- [ ] Create priority inversion on purpose with a semaphore, then fix it with a mutex.
- [ ] Enable `configCHECK_FOR_STACK_OVERFLOW` and the high-water marks, then trim stack sizes.
- [ ] Trace with SEGGER SystemView and measure the control task's jitter.

**Done when:** the control loop's jitter stays under about 10 µs while the shell and telemetry are busy.

## Phase 5 — Sensors and actuators (3–4 weeks)

Suggested add-ons: MPU6050 (ATK module or breakout), a 12 V DC gear motor with
a quadrature encoder, a TB6612FNG or L298N driver, an SG90 servo, a 28BYJ-48 stepper.

- [ ] MPU6050 over I2C: read the raw values, calibrate the gyro bias, compute tilt from the accelerometer.
- [ ] Complementary filter for pitch. Then a 1D Kalman filter. Compare them.
- [ ] Servo PWM (50 Hz, 1–2 ms pulses) following the IMU angle.
- [ ] DC motor: PWM at 20 kHz (inaudible), direction pins, encoder speed measurement.
- [ ] Stepper with acceleration ramps (timer-driven step pulses).
- [ ] DS18B20 on PG9: one-wire timing with microsecond delays (DWT cycle counter).

## Phase 6 — Control algorithms (ongoing)

Theory: Brian Douglas's control videos; *Feedback Systems* (Åström & Murray) chapters 1–6, 10–11.

- [ ] Run `s` on the board and plot it (`make plot`). Then change the gains in `run_step_sim()`:
      - Why does `ki/kp = 1/tau` give a clean first-order response with no overshoot?
      - Increase `kp` until the step overshoots. Then add plant dead time and see when it goes unstable.
- [ ] **System identification:** apply a PWM step to the real motor, log the speed, and fit `K` and `tau` (`tools/` + numpy).
- [ ] **Motor speed PID** at 1 kHz using the fitted model to pick the gains. Compare the measured step response with the simulation.
- [ ] **Cascaded position control:** an inner speed loop (1 kHz) and an outer position loop (200 Hz).
- [ ] **Light-level control:** LED PWM → LDR (PF7) → PID. A whole closed loop using only on-board parts.
- [ ] **Temperature control:** a resistor heater + DS18B20. It's a slow plant, so integral windup really matters.
- [ ] Capstone: a self-balancing robot (IMU fusion + cascaded PID or LQR + FreeRTOS).
- [ ] Stretch: state-space / LQR design in Python (`python-control`), implemented with the CMSIS-DSP matrix functions.

## Reference shelf
- *Making Embedded Systems*, Elecia White
- *The Definitive Guide to ARM Cortex-M3 and M4*, Joseph Yiu
- *Mastering STM32*, Carmine Noviello
- *Mastering the FreeRTOS Real Time Kernel* (free PDF, freertos.org)
- *Feedback Systems*, Åström & Murray (free PDF)
- "PID Without a PhD", Tim Wescott (free article)
- Miro Samek, *Modern Embedded Systems Programming* (YouTube course)
- ALIENTEK 探索者 开发指南: the board vendor's step-by-step examples for every on-board peripheral
