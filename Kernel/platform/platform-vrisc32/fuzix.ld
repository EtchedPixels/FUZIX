SEARCH_DIR(.)

MEMORY
{
  ram (rwx) : ORIGIN = 0x3FC80000, LENGTH = 0x80000
}

/*
 * stick everything in ram (of course)
 */
SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text .text.*)

    . = ALIGN(0x10);

    *(.rodata .rodata.*)

    . = ALIGN(0x4);
    _etext = .;
    *(.lit)
  } > ram

  .data :
  {
    . = ALIGN(0x10);
    PROVIDE(__global_pointer$ = .+ 0x800);
    
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    _edata = .;
  } > ram

  .bss :
  {
    . = ALIGN(0x10);
    __bss_start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _end =  ALIGN (0x10);
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
