/*
 * Linker script for STM32F407ZG (1 MB flash, 128 KB SRAM + 64 KB CCM RAM).
 *
 * Read it alongside startup_stm32f407.c: the symbols defined here
 * (_sidata, _sdata, _ebss, _estack, ...) are what the reset handler uses
 * to copy .data from flash to RAM and zero .bss before main() runs.
 */
ENTRY(Reset_Handler)

_min_heap_size  = 0x2000;  /* 8 KB, only checked; newlib malloc uses it */
_min_stack_size = 0x1000;  /* 4 KB main stack (MSP) */

MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM    (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCMRAM (rw)  : ORIGIN = 0x10000000, LENGTH = 64K   /* CPU-only: no DMA! */
}

/* The stack grows down from the top of SRAM. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Vector table must sit at the start of flash (address 0 alias at boot). */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7) *(.glue_7t) *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* Constructor tables, walked by the reset handler. */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH
    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    /* Initialised variables: live in RAM (VMA), stored in flash (LMA). */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    /* Zero-initialised variables. */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Put fast, non-DMA buffers here with __attribute__((section(".ccmram"))).
     * Not initialised by the startup code: treat contents as garbage. */
    .ccmram (NOLOAD) :
    {
        . = ALIGN(4);
        *(.ccmram)
        *(.ccmram*)
        . = ALIGN(4);
    } > CCMRAM

    /* Fails the link if heap + stack no longer fit after .bss. */
    ._user_heap_stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _min_heap_size;
        . = . + _min_stack_size;
        . = ALIGN(8);
    } > RAM

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
