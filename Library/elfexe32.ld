ENTRY (_start)

PHDRS {
	text PT_LOAD;
	data PT_LOAD;
}

SECTIONS {
	.text : {
		__text_start = .;
		LONG(__sighandler);
		*(.header)
		*(.literal .text)
		*(.literal.* .text.*)
		*(.rodata)
		*(.rodata1)
		*(.rodata.*)
		__text_end = .;
	} : text = 0xff

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
	}

	/DISCARD/ : { *(.comment .note) }
}
