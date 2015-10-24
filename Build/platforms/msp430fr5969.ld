#include "msp430fr5969.ld.h"

ENTRY(_start)

MEMORY
{
        USER : ORIGIN = USER_BASE, LENGTH = (0xff80 - USER_BASE)
}

SECTIONS
{
	.image : {
		__image_start = .;
		*(.header*)
		*(.text*)
		. = ALIGN(2);
		__data_start = .;
		*(.data*)
		*(.rodata*)
		*(.eh_frame*)
		__data_end = .;
	} > USER
	__data_len = (__data_end - __data_start);
	__user_page = (__image_start / 0x100);

	.bss : {
		. = ALIGN(2);
		__bss_start = .;
		*(.bss* COMMON)
		__bss_end = .;
	} > USER
	__bss_len = (__bss_end - __bss_start);

	/* The rest are all not normally part of the runtime image.  */

	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }

	/* DWARF debug sections.
	 Symbols in the DWARF debugging sections are relative to the beginning
	 of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	/DISCARD/ : { *(.note.GNU-stack) }
}

