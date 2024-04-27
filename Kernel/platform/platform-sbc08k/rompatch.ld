OUTPUT_ARCH(m68k)

SEARCH_DIR(.)

MEMORY
{
  rom (rx) : ORIGIN = 0xE3F00, LENGTH = 0x00100
  ram (rwx) : ORIGIN = 0x0300, LENGTH = 0x10
}

/*
 * stick everything in ROM
 */
SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text .text.*)
    . = ALIGN(0x4);
    *(.rodata .rodata.*)
    _etext = .;
    *(.lit)
  } > rom

  .data :
  {
    _data = .;
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    _edata = .;
  } > ram AT >rom

  .bss :
  {
    . = ALIGN(0x4);
    __bss_start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _end =  ALIGN (0x8);
    __end = _end;
  } > ram

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
}
