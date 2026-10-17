# Convenience wrapper. Run `make help` to see the targets.

BUILD_DIR ?= build
BUILD_TYPE ?= Debug
SERIAL ?= /dev/ttyUSB0

.PHONY: all firmware test phase1 flash flash-isp monitor plot clean help

all: firmware

firmware:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR) -j

test:
	$(MAKE) -C tests

phase1:
	@$(MAKE) -s -C tests phase1

# SWD via ST-LINK on the 20-pin JTAG header.
flash: firmware
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $(BUILD_DIR)/firmware.elf verify reset exit"

# Serial bootloader via the CH340 on USB_232 (set BOOT0=1, BOOT1=0, press RESET first).
flash-isp: firmware
	stm32flash -b 115200 -w $(BUILD_DIR)/firmware.bin -v -g 0x08000000 $(SERIAL)

monitor:
	python3 -m serial.tools.miniterm $(SERIAL) 115200

plot:
	python3 tools/plot_telemetry.py --port $(SERIAL) --command s

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C tests clean

help:
	@echo "make firmware    build build/firmware.{elf,bin,hex}"
	@echo "make test        build and run host-side unit tests (lib/)"
	@echo "make phase1      phase 1 exercise scoreboard"
	@echo "make flash       program over SWD with an ST-LINK + OpenOCD"
	@echo "make flash-isp   program over the USB_232 port (stm32flash)"
	@echo "make monitor     serial terminal on SERIAL=$(SERIAL)"
	@echo "make plot        run the PID simulation on the board and plot it"
