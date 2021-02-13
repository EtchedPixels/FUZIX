ENTRY (_start)

PHDRS {
	text PT_LOAD;
	data PT_LOAD;
	noload PT_LOAD FLAGS(0);
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

	.rel.dyn : { *(.rel.dyn) } : noload 
	.dynamic : { *(.dynamic) } : noload
	.dynsym : { *(.dynsym) } : noload

	/DISCARD/ : { *(.comment .note) }
}
