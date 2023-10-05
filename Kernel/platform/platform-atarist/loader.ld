OUTPUT_ARCH(m68k)

SEARCH_DIR(.)

MEMORY
{
  /* Really it's undefined but we need to relocate ourself somewhere
     above the kernel to be safe. All systems have at least 512K of
     RAM so 192K is a fine spot to sit */
  ram (rwx) : ORIGIN = 0x30000, LENGTH = 0x200
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
    _etext = .;
  } > ram
}
