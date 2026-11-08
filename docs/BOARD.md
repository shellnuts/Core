# ALIENTEK Explorer (探索者) STM32F4 — board notes

**MCU:** STM32F407ZGT6. Cortex-M4F at up to 168 MHz, 1 MB flash, 128 KB SRAM plus 64 KB CCM RAM, LQFP-144.
**Crystals:** 8 MHz HSE, 32.768 kHz LSE (RTC, with a CR1220 backup battery).

> Pin numbers come from the ALIENTEK schematic V2.2 in [datasheet/](datasheet/).
> Other board revisions differ slightly. If your board has a different revision,
> download its schematic from ALIENTEK (openedv.com, "探索者 STM32F407 资料")
> and check it before you wire anything external.

## Key documents (download these first)
| Document | What it's for |
|---|---|
| **RM0090** – STM32F405/407 reference manual | How every peripheral works, register by register |
| **STM32F407xx datasheet** (DS8626) | Pinout, alternate-function table, electrical limits |
| **PM0214** – Cortex-M4 programming manual | Core, NVIC, SysTick, FPU, fault registers |
| **ES0182** – STM32F40x errata | Silicon bugs. Read it before blaming your code |
| ALIENTEK Explorer [schematic V2.2](datasheet/Explorer%20STM32F4_V2.2_SCH.pdf) + [开发指南 V1.1 (dev guide)](<datasheet/STM32F4开发指南-库函数版本_V1.1.pdf>) | Board wiring, example projects. The dev guide is in Chinese and uses ST's Standard Peripheral Library |

The two ALIENTEK PDFs are in `docs/datasheet/`, which git ignores because the dev guide is 43 MB. On a new clone, download them from openedv.com.

## On-board resources and the phase that uses them

| Resource | MCU pins / peripheral | Learning use |
|---|---|---|
| LED DS0 (red), DS1 (green) | PF9, PF10, active **low** | Phase 2: GPIO output, timer PWM brightness (TIM14_CH1 on PF9) |
| KEY0 / KEY1 / KEY2 | PE4 / PE3 / PE2, active low (pull-up) | GPIO input, debouncing, EXTI interrupts |
| WK_UP key | PA0, active **high** (pull-down) | EXTI, wake from standby |
| Buzzer | PF8, active high | GPIO, PWM tones |
| Power switch K1 | — | Turns the whole board on and off. USB_232 supplies 5 V through a 1000 mA fuse, and the DC jack is the other source |
| USB_232 (CH340) | USART1 PA9 TX / PA10 RX | printf, command shell, telemetry, one-key serial flashing |
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
**Make sure the USART1 jumpers on P6 (PA9 ↔ RXD, PA10 ↔ TXD) are fitted** so the CH340 reaches the MCU.

PB3 and PB4 are JTAG pins (JTDO, JTRST) and also SPI1 pins for the W25Q128. Debug with SWD only, which is the OpenOCD `stlink.cfg` default, so that SPI1 can use them. PB2 is BOOT1 and also the touch-screen T_MISO line.

## Programming and debugging

The board has **no built-in debugger**. You have two options:

1. **ST-LINK V2 on the JTAG/SWD header (recommended).** A $5 clone works. Connect SWDIO, SWCLK, GND and 3.3 V reference, or use a 20-pin ribbon adapter. This gives you breakpoints, register views and fault analysis, which you'll need from phase 2 onwards.
   ```
   make flash          # OpenOCD
   ```
   STM32CubeProgrammer and STM32CubeIDE also work with the same probe.

2. **Serial bootloader over USB_232 (no extra hardware).** The board has a one-key download circuit. Transistors Q3 and Q4 let the CH340's DTR and RTS lines drive RESET and BOOT0, so you do not move a jumper. Keep both BOOT jumpers on GND (BOOT0 = 0, BOOT1 = 0).
   - On Windows, use ALIENTEK's FlyMcu. Select "DTR的低电平复位，RTS高电平进BootLoader" (DTR low resets, RTS high enters the bootloader). Do not select RamIsp. After the download, your program starts by itself. Dev guide sections 2.1.31 and 3.4.1 describe this.
   - A serial download erases the whole flash first. On the STM32F4, this takes tens of seconds.
   - If a tool cannot toggle DTR and RTS (for example STM32CubeProgrammer in UART mode), set BOOT0 = 1 by hand and press RESET. Then flash, set BOOT0 back to 0 and press RESET to run your program:

     ```bash
     make flash-isp SERIAL=/dev/ttyUSB0      # Linux/macOS, needs stm32flash
     ```

   You cannot debug this way. You can only flash and use printf.

Drivers: install the **CH340** USB-serial driver on Windows/macOS. Linux has it built in.
