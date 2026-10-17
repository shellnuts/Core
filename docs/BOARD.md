# ALIENTEK Explorer (探索者) STM32F4 — board notes

**MCU:** STM32F407ZGT6. Cortex-M4F at up to 168 MHz, 1 MB flash, 128 KB SRAM plus 64 KB CCM RAM, LQFP-144.
**Crystals:** 8 MHz HSE, 32.768 kHz LSE (RTC, with a CR1220 backup battery).

> Pin numbers come from ALIENTEK's published schematic for this board.
> Revisions differ slightly. Download the schematic for your revision from
> ALIENTEK (openedv.com, "探索者 STM32F407 资料") and check it before wiring
> anything external.

## Key documents (download these first)
| Document | What it's for |
|---|---|
| **RM0090** – STM32F405/407 reference manual | How every peripheral works, register by register |
| **STM32F407xx datasheet** (DS8626) | Pinout, alternate-function table, electrical limits |
| **PM0214** – Cortex-M4 programming manual | Core, NVIC, SysTick, FPU, fault registers |
| **ES0182** – STM32F40x errata | Silicon bugs. Read it before blaming your code |
| ALIENTEK Explorer schematic + 开发指南 (dev guide) | Board wiring, example projects |

## On-board resources and the phase that uses them

| Resource | MCU pins / peripheral | Learning use |
|---|---|---|
| LED DS0 (red), DS1 (green) | PF9, PF10, active **low** | Phase 2: GPIO output, timer PWM brightness (TIM14_CH1 on PF9) |
| KEY0 / KEY1 / KEY2 | PE4 / PE3 / PE2, active low (pull-up) | GPIO input, debouncing, EXTI interrupts |
| WK_UP key | PA0, active **high** (pull-down) | EXTI, wake from standby |
| Buzzer | PF8, active high | GPIO, PWM tones |
| USB_232 (CH340) | USART1 PA9 TX / PA10 RX | printf, command shell, telemetry, ISP flashing |
| RS232 / RS485 / CAN | USART2/3, CAN1 (jumper-selected) | Phase 3+: industrial buses |
| 24C02 EEPROM | I2C: PB8 SCL, PB9 SDA | Phase 3: I2C driver, storing PID gains |
| W25Q128 16 MB SPI flash | SPI1 PB3/PB4/PB5, CS PB14 | Phase 3: SPI + DMA, data logging |
| Light sensor (LDR) | PF7 = ADC3_IN5 | Phase 3: ADC, filtering |
| DAC output | PA4 = DAC_OUT1 | Generate test signals, closed-loop with ADC |
| IR receiver | PA8 (TIM1_CH1) | Timer input capture (NEC decoding with the remote) |
| DS18B20 / DHT11 header | PG9 | One-wire timing, temperature control project |
| ATK MODULE header | USART3 (PB10/PB11) | MPU6050 / GPS / Bluetooth modules |
| TFT LCD header, 1 MB SRAM | FSMC | Phase 5+: display, external memory |
| Ethernet (LAN8720) | RMII | Optional, later |
| USB OTG FS (USB_SLAVE) | PA11/PA12 | USB CDC: faster telemetry than UART |
| JTAG/SWD 20-pin header | PA13 SWDIO, PA14 SWCLK | Debugging with an ST-LINK |

Some pins are shared, for example PA11/PA12 (USB and CAN) and the RS232/RS485 USARTs. The jumper blocks on the board select which function connects.
**Make sure the USART1 jumpers (PA9 ↔ RXD, PA10 ↔ TXD) are fitted** so the CH340 reaches the MCU.

## Programming and debugging

The board has **no built-in debugger**. You have two options:

1. **ST-LINK V2 on the JTAG/SWD header (recommended).** A $5 clone works. Connect SWDIO, SWCLK, GND and 3.3 V reference, or use a 20-pin ribbon adapter. This gives you breakpoints, register views and fault analysis, which you'll need from phase 2 onwards.
   ```
   make flash          # OpenOCD
   ```
   STM32CubeProgrammer and STM32CubeIDE also work with the same probe.

2. **Serial bootloader over USB_232 (no extra hardware).** Set the BOOT jumpers to **BOOT0 = 1, BOOT1 = 0**, press RESET, then:
   ```
   make flash-isp SERIAL=/dev/ttyUSB0      # Linux/macOS, needs stm32flash
   ```
   On Windows, use STM32CubeProgrammer (UART mode) or ALIENTEK's FlyMcu.
   Afterwards, set BOOT0 back to 0 and press RESET to run your program.
   You can't debug this way, only flash and use printf.

Drivers: install the **CH340** USB-serial driver on Windows/macOS. Linux has it built in.
