/*
 * GNU linker script for Espressif ESP32
 */

/* Default entry point */
ENTRY(__pro_cpu_entry);

/* Specify main memory areas */
MEMORY
{
  /* Use values from the ESP-IDF 'bootloader' component.
  /* TODO: Use human-readable lengths */
  /* TODO: Use the full memory map - this is just a test */
  iram_seg ( RX )       : ORIGIN = 0x400a0000, len = 0x20000
  dram_seg ( RW )       : ORIGIN = 0x3FFce000, len = 72*1024
}

_code_base = 0x40080000;
_code_top = 0x400a0000;
_data_base = 0x3ffc0000;
_data_top = 0x3ffe0000;

/* Define output sections */
SECTIONS {
  /* The program code and other data goes into Instruction RAM */
  .iram.text :
  {
    . = ALIGN(16);
    KEEP(*(.entry.text))
    *(.text)
    *(.text*)
    KEEP (*(.init))
    KEEP (*(.fini))
    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);
    _etext = .;
  } >iram_seg

  /* Initialized data goes into Data RAM */
  _sidata = .;
  .data : AT(_sidata)
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;
  } >dram_seg

  /* Uninitialized data also goes into Data RAM */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
  } >dram_seg

  . = ALIGN(4);
  PROVIDE ( end = . );
  PROVIDE ( _end = . );
}
