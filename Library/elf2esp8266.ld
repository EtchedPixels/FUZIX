ENTRY (_start)

MEMORY {
	data : ORIGIN = 0x3ffe8600, len = 0xfa00
	code : ORIGIN = 0x40100000, len = 0x7e00
}

PHDRS {
	text PT_LOAD;
	data PT_LOAD;
	bss PT_LOAD;
}

SECTIONS {

	.text ORIGIN(code) : {
		__text_start = .;
		SHORT(0x80a8); 	/* Fuzix executable */
		BYTE(11);       /* LX106 CPU */
		BYTE(1);        /* ESP8266 */
		BYTE(0);        /* Base page; meaningless */
		BYTE(0);        /* No hints */

		SHORT(__text_end - __text_start)
		SHORT(__data_end - __data_start)
		SHORT(__bss_end - __bss_start)

		BYTE(_start - __text_start); /* offset to entrypoint */
		BYTE(0);        /* No size hint */
		BYTE(0);        /* No stack hint */
		BYTE(0);        /* No ZP hint */

		LONG(__sighandler);

		*(.header)
		*(.literal .text)
		*(.literal.* .text.*)
		. = ALIGN(4);
		__text_end = .;
	} > code : text

	.data ORIGIN(data) : {
		__data_start = .;

		*(.rodata)
		*(.rodata1)
		*(.rodata.*)

		*(.data)
		*(.data1)
		*(.data.*)
		__data_end = .;
	} > data : data

	.bss ALIGN(4) (NOLOAD) : {
		__bss_start = .;
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.bss*)
		*(COMMON)
		__bss_end = .;
	} > data : bss

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
}
