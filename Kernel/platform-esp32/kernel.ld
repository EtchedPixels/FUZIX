/*
 * GNU linker script for Espressif ESP32
 */

/* Default entry point */
ENTRY(__relocate);

/* Specify main memory areas */
MEMORY
{
    sram2(RW):  ORIGIN = 0x3ffae000, len = 72*1024
    pdata(RW):  ORIGIN = 0x3ffc0000, len = 128*1024
    sram1d(RW): ORIGIN = 0x3ffe0000, len = 128*1024

    sram0(RWX): ORIGIN = 0x40070000, len = 64*1024
    pcode(RWX): ORIGIN = 0x40080000, len = 128*1024
    sram1c(RWX): ORIGIN = 0x400a0000, len = 128*1024
}

_code_base = 0x40080000;
_code_top = 0x400a0000;
_data_base = 0x3ffc0000;
_data_top = 0x3ffe0000;

SECTIONS
{
    .relocate :
    {
        *(.text.relocate)
        . = ALIGN(4);
    } >pcode

    .text :
    {
        . = ALIGN(4);
        _stext_start = LOADADDR(.text);
        _text_start = .;
        KEEP(*(.entry.text))
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
    } >sram2 AT>pcode

    .bss ALIGN(4) (NOLOAD) :
    {
        _bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _bss_end = .;
    } >sram2
}

/* vim: set sw=4 ts=4 et: */
