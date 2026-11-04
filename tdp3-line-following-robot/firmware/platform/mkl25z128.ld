/*
 * Linker script for the MKL25Z128VLK4 (128 KB flash, 16 KB RAM).
 *
 * Flash layout:
 *   0x00000000  vector table (48 words)
 *   0x00000400  flash configuration field (16 bytes, must sit exactly here)
 *   0x00000410  code + constant data + initial values for .data
 *
 * RAM: SRAM_L (0x1FFFF000, 4 KB) and SRAM_U (0x20000000, 12 KB) are
 * contiguous, so they are used as one 16 KB block. The stack grows down
 * from the top of RAM.
 */

MEMORY
{
    VECTORS  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00000400
    FLASHCFG (r)   : ORIGIN = 0x00000400, LENGTH = 0x00000010
    FLASH    (rx)  : ORIGIN = 0x00000410, LENGTH = 0x0001FBF0
    RAM      (rw)  : ORIGIN = 0x1FFFF000, LENGTH = 0x00004000
}

ENTRY(Reset_Handler)

__StackTop       = ORIGIN(RAM) + LENGTH(RAM);
__min_stack_size = 0x800;   /* 2 KB reserved for the stack */

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > VECTORS

    .flash_config :
    {
        KEEP(*(.flash_config))
    } > FLASHCFG

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    _sidata = LOADADDR(.data);

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

    ASSERT(_ebss + __min_stack_size <= __StackTop, "Out of RAM: not enough room left for the stack")
}
