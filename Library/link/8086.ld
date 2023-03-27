/* Default linker script, for normal executables */
/* Copyright (C) 2014-2017 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT("msdos")
OUTPUT_ARCH(i386)
SEARCH_DIR("/opt/gcc-ia16/ia16-elf/lib");
SECTIONS
{
  . = 0x0;
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text)
    etext = .;
    _etext = .;
    __etext = .;
  }
  .data :
  {
    *(.rodata)
    *(.data)
    CONSTRUCTORS
    edata  =  .;
    _edata  =  .;
    __edata  =  .;
  }
  .bss :
  {
    _bss_start = .;
    __bss_start = .;
   *(.bss)
   *(COMMON)
   end = ALIGN(4) ;
   _end = ALIGN(4) ;
   __end = ALIGN(4) ;
  }
}
