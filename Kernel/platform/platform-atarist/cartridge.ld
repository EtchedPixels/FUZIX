OUTPUT_FORMAT("binary")

MEMORY
{
	cartridge(rx) : ORIGIN = 0xfa0000 - 4, LENGTH = 128K + 4
}

SECTIONS
{
	.cartridge : ALIGN(4) {
		LONG(0)				/* For the Hatari emulator */
		LONG(0xabcdef42)		/* CA_MAGIC */
		LONG(0)				/* CA_NEXT or NULL */
		LONG(_ca_run | 0x01000000) 	/* CA_INIT */
		LONG(_ca_run)			/* CA_RUN */
		SHORT(0)			/* CA_TIME */
		SHORT(0)			/* CA_DATE */
		LONG(_ca_end - _ca_run)		/* CA_SIZE */
		KEEP(*(.filename))		/* CA_NAME */
	} >cartridge

	_ca_run = ALIGN(4) ;

	.text : {
		*(.text)
		*(.text.*)
	} >cartridge = 0

	.rodata : ALIGN(4) {
		*(.rodata)
		*(.rodata.*)
	} >cartridge = 0

	.data : ALIGN(4) {
		*(.data)
		*(.data.*)
	} >cartridge = 0

	_ca_end = . ;

	.zero : {
		BYTE(0)
		. = 0xfc0000 - _ca_end ;
	} >cartridge = 0

	/DISCARD/ : {
		*(.comment)
		*(.debug)
		*(.bss)
	}
}
