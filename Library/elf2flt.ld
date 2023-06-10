
ENTRY (_start)

MEMORY {
	/* 0x20 so we allow for the stubs */
	flatmem : ORIGIN = 0x20, LENGTH = 0xfffffff
}

PHDRS {
	text PT_LOAD ;
	data PT_LOAD ;
}

SECTIONS {

	.text : {
		__stubs = . - 0x20;
		. = . + 4;
		. = ALIGN(0x4) ;
		_stext = . ;
		*(.literal .text)
		*(.literal.* .text.*)
		*(.gnu.warning)
		*(.stub)
		*(.gnu.linkonce.literal.*)
		*(.gnu.linkonce.t*)
		*(.glue_7t)
		*(.glue_7)
		*(.jcr)
		*(.init.literal)
		KEEP (*(.init))
		*(.fini.literal)
		KEEP (*(.fini))

		*(.rodata)
		*(.rodata1)
		*(.rodata.*)
		*(.gnu.linkonce.r*)

		/* .ARM.extab name sections containing exception unwinding information */
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		/* This is special code area at the end of the normal
		   text section.  It contains a small lookup table at
		   the start followed by the code pointed to by entries
		   in the lookup table.  */
		. = ALIGN (4) ;
		PROVIDE(__ctbp = .);
		*(.call_table_data)
		*(.call_table_text)
	} > flatmem :text

	/* .ARM.exidx name sections containing index entries for section unwinding */
	/* .ARM.exidx is sorted, so has to go in its own output section.  */
	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > flatmem
	__exidx_end = .;

	_etext = . ;

	.data : {
		. = ALIGN(0x4) ;
		_sdata = . ;
		__data_start = . ;
		data_start = . ;
		*(.got.plt)
		*(.got)
		FILL(0) ;
		. = ALIGN(0x20) ;
		LONG(-1)
		. = ALIGN(0x20) ;
		*(.data)
		*(.data1)
		*(.data.*)
		*(.gnu.linkonce.d*)

		/* Microblaze has .sdata and .sbss (small bss).  They must
		   be contiguous, so please don't move any of this. JW */
		_ssrw = . ;
		*(.sdata)
		*(.sdata.*)
		*(.sbss)			/* Don't move this! */
		*(.gnu.linkonce.sb*)
		_essrw = . ;

		_ssrw_size = _essrw - _ssrw;
		PROVIDE(_SDA_BASE_ = _ssrw + (_ssrw_size / 2));

		*(.gnu.linkonce.s.*)
		*(__libc_atexit)
		*(__libc_subinit)
		*(__libc_subfreeres)

		/* microblaze-specific read-only small data area
		   and associated locating symbols */
		_ssro = . ;
		*(.sdata2)
		_essro = . ;
		_ssro_size = _essro - _ssro;
		PROVIDE(_SDA2_BASE_ = _ssro + (_ssro_size / 2));

		. = ALIGN(4) ;
TOR:		__CTOR_LIST__ = .;
TOR:		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
SINGLE_LINK:	/* gcc uses crtbegin.o to find the start of
SINGLE_LINK:	   the constructors, so we make sure it is
SINGLE_LINK:	   first.  Because this is a wildcard, it
SINGLE_LINK:	   doesn't matter if the user does not
SINGLE_LINK:	   actually link against crtbegin.o; the
SINGLE_LINK:	   linker won't look for a file to match a
SINGLE_LINK:	   wildcard.  The wildcard also means that it
SINGLE_LINK:	   doesn't matter which directory crtbegin.o
SINGLE_LINK:	   is in.  */
SINGLE_LINK:	KEEP (*crtbegin*.o(.ctors))
SINGLE_LINK:	/* We don't want to include the .ctor section from
SINGLE_LINK:	   from the crtend.o file until after the sorted ctors.
SINGLE_LINK:	   The .ctor section from the crtend file contains the
SINGLE_LINK:	   end of ctors marker and it must be last */
SINGLE_LINK:	KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
SINGLE_LINK:	KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
TOR:		LONG(0)
TOR:		__CTOR_END__ = .;
TOR:		__DTOR_LIST__ = .;
TOR:		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
SINGLE_LINK:	KEEP (*crtbegin*.o(.dtors))
SINGLE_LINK:	KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
SINGLE_LINK:	KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
TOR:		LONG(0)
TOR:		__DTOR_END__ = .;

		PROVIDE (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE (__preinit_array_end = .);

		PROVIDE (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		PROVIDE (__init_array_end = .);

		PROVIDE (__fini_array_start = .);
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		PROVIDE (__fini_array_end = .);
	} > flatmem :data

	.note.ABI-tag : { *(.note.ABI-tag) } > flatmem
	.eh_frame_hdr : { *(.eh_frame_hdr) } > flatmem
	.eh_frame : { KEEP(*(.eh_frame)) } > flatmem
	.gcc_except_table : {
		KEEP(*(.gcc_except_table))
		KEEP(*(.gcc_except_table.*))
	} >flatmem

	. = ALIGN(0x10) ;
	_edata = . ;

	.bss : {
		. = ALIGN(0x4) ;
		_sbss = ALIGN(0x4) ;
		__bss_start = . ;
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.bss*)
		*(.gnu.linkonce.b*)
		*(COMMON)
		. = ALIGN(0x10) ;
		_ebss = . ;
		_end = . ;
		end = . ;
		__bss_end = . ;
	} > flatmem

	.stack : {
		. = ALIGN(0x4);
		_stack_start = .;
	} > flatmem
}
