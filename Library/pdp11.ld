/* Script for -n: mix text and data on same page */
/* Copyright (C) 2014-2018 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT("a.out-pdp11", "a.out-pdp11",
	      "a.out-pdp11")
OUTPUT_ARCH(pdp11)
SEARCH_DIR("/opt/pdp11test/pdp11-aout/lib");
PROVIDE (__stack = 0);

SECTIONS
{
  . = 0;
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text)
    /* The next six sections are for SunOS dynamic linking.  The order
       is important.  */
    *(.dynrel)
    *(.hash)
    *(.dynsym)
    *(.dynstr)
    *(.rules)
    *(.need)
    _etext = .;
    __etext = .;
    . = ALIGN(2);
  }
  .data :
  {
    /* The first three sections are for SunOS dynamic linking.  */
    *(.dynamic)
    *(.got)
    *(.plt)
    *(.data)
    *(.linux-dynamic) /* For Linux dynamic linking.  */
    CONSTRUCTORS
    _edata  =  .;
    __edata  =  .;
    . = ALIGN(2);
  }
  .bss :
  {
    __bss_start = .;
   *(.bss)
   *(COMMON)
   . = ALIGN(2);
   _end = . ;
   __end = . ;
  }
}
