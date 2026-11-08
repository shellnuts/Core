cmake_minimum_required(VERSION 3.20)

# Configure with: cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
project(explorer_f407 C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/firmware/linker/stm32f407zg.ld)

add_executable(firmware
    firmware/startup/startup_stm32f407.c
    firmware/src/main.c
    firmware/src/syscalls.c
    firmware/drivers/clock.c
    firmware/drivers/systick.c
    firmware/drivers/gpio.c
    firmware/drivers/uart.c
    firmware/board/board.c
    lib/ringbuf.c
    lib/pid.c
)
set_target_properties(firmware PROPERTIES SUFFIX .elf)

target_include_directories(firmware PRIVATE
    firmware/include
    firmware/drivers
    firmware/board
    lib
)

target_compile_options(firmware PRIVATE
    ${MCU_FLAGS}
    -Wall -Wextra -Wshadow -Wdouble-promotion
    -fdata-sections -ffunction-sections
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-O2 -g>
)

target_link_options(firmware PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -nostartfiles
    --specs=nano.specs
    -u _printf_float          # let printf handle %f (costs ~10 KB flash)
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map,--cref
    -Wl,--print-memory-usage
)
target_link_libraries(firmware PRIVATE m)
set_target_properties(firmware PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware> ${CMAKE_BINARY_DIR}/firmware.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:firmware> ${CMAKE_BINARY_DIR}/firmware.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
    COMMENT "Creating firmware.bin / firmware.hex"
)
