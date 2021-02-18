ENTRY (_start)

PHDRS {
	dynamic PT_DYNAMIC;
	text PT_LOAD;
	data PT_LOAD;
	info PT_LOAD;
}

SECTIONS {
	.text ALIGN(4) : {
		__text_start = .;
		LONG(__sighandler);
		*(.literal .text)
		*(.literal.* .text.*)
		*(.rodata)
		*(.rodata1)
		*(.rodata.*)
		__text_end = .;
	} : text

	.got ALIGN(4) : { *(.got*) } : text
	.plt ALIGN(4) : { *(.plt*) } : text
	.ARM.exidx ALIGN(4) : { *(.ARM.exidx) } : text

	.data ALIGN(4) : {
		__data_start = .;
		*(.data)
		*(.data1)
		*(.data.*)
		__data_end = .;
	} : data

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
	} : data

	.dynamic : { *(.dynamic) } :dynamic :info
	.dynsym : { *(.dynsym) } :info
	.rel.dyn : { *(.rel*) } :info
	.hash : { *(.hash*) } :info

	/DISCARD/ : { *(.comment .note) }
}
