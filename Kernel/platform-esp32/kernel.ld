/*
 * GNU linker script for Espressif ESP32
 */

/* Default entry point */
ENTRY(__startup);

/* Specify main memory areas */
MEMORY
{
    sram2(RW):  ORIGIN = 0x3ffae000, len = 72*1024
    pdata(RW):  ORIGIN = 0x3ffc0000, len = 128*1024
    /* sram1d(RW): ORIGIN = 0x3ffe0000, len = 128*1024 */

    sram0(RWX): ORIGIN = 0x40070000, len = 64*1024
    pcode(RWX): ORIGIN = 0x40080000, len = 128*1024
    sram1c(RWX): ORIGIN = 0x400a0000, len = 128*1024
}

_code_base = 0x40080000;
_code_top = 0x400a0000;
_data_base = 0x3ffc0000;
_data_top = 0x3ffe0000;

udata = _data_base;
_data_app_start = udata + 0x400;

kputchar = ets_write_char_uart;

SECTIONS
{
    .relocate :
    {
        startup_asm.o(.text*)
        startup.o(.text*)
        . = ALIGN(4);
    } >pcode

    .text :
    {
        . = ALIGN(4);
        _stext_start = LOADADDR(.text);
        _text_start = .;

        /* The vector table goes at the very beginning of memory as a cheap way
         * to get the alignment right. */

        _platform_exception_table = .;
        . = 0x0;   KEEP(*(.text.WindowOverflow4));
        . = 0x40;  KEEP(*(.text.WindowUnderflow4));
        . = 0x80;  KEEP(*(.text.WindowOverflow8));
        . = 0xc0;  KEEP(*(.text.WindowUnderflow8));
        . = 0x100; KEEP(*(.text.WindowOverflow12));
        . = 0x140; KEEP(*(.text.WindowUnderflow12));
        . = 0x180; KEEP(*(.text.Level2InterruptVector));
        . = 0x1c0; KEEP(*(.text.Level3InterruptVector));
        . = 0x200; KEEP(*(.text.Level4InterruptVector));
        . = 0x240; KEEP(*(.text.Level5InterruptVector));
        . = 0x280; KEEP(*(.text.DebugExceptionVector));
        . = 0x2c0; KEEP(*(.text.NMIExceptionVector));
        . = 0x300; KEEP(*(.text.KernelExceptionVector));
        . = 0x340; KEEP(*(.text.UserExceptionVector));
        . = 0x3C0; KEEP(*(.text.DoubleExceptionVector));

        *(.text)
        *(.text*)
        . = ALIGN(4);
        _text_end = .;
    } >sram0 AT>pcode

    .data :
    {
        . = ALIGN(4);
        _sdata_start = LOADADDR(.data);
        _data_start = .;
        *(.rodata)
        *(.rodata*)
        *(.data)
        *(.data*)
        *(.sdata)
        *(.sdata*)
        . = ALIGN(4);
        _data_end = .;
    } >pdata AT>pcode

    .bss ALIGN(4) (NOLOAD) :
    {
        _bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _bss_end = .;
    } >pdata
}

/* vim: set sw=4 ts=4 et: */
