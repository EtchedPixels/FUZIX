;
;	We are loaded from the boot block by the firmware and are somewhere
;	in the original mapping 0 low, 1 high at E000. Stack is at FExx
;	somewhere.
;

	.code
#if 0
	FIXME - why does this cause a linker error
	.byte 'S'
	.byte 8
#endif

start:
;
;	Fix up the MMU mappings so we have RAM low as well
;
	di			; We are about to map out the irqvecs
	ld	r13,#0x11
	and	2,#0xBF		; I/O on for upper data space
	lde	0xFFFF,r13
	or	2,#0x10
	;	We now have lowest ram bank mapped for all of memory
	;	Load the code segment
	clr	r14
	clr	r15
	ld	r13,#112	; load 56K of blocks
	call	load_data
	;
	;	Now switch the data bank for our new data bank
	;
	ld	r13,#0x31
	and	2,#0xBF
	lde	0xFFFF,r13
	or	2,#0x10
	;
	;	Now load the data segment
	;
	clr	r14
	clr	r15
	ld	r13,#112	; Will be plenty
	call	load_data
	;
	;	Switch the maps and call the program
	;
	and	2,#0xBF
	ld	r13,#0x33	; Set data
	lde	0xFFFF,r13
	ld	r13,#0x11	; Set code
	lde	0xFFFE,r13
	or	2,#0x10
	;
	;	We now have code mapping of
	;	0000-DFFF : code	RAM bank 0
	;	E000-FFFF : loader etc	RAM bank 0
	;	Data mapping of
	;	0000-7FFF : data	RAM bank 1
	;	E000-FFFF : data	RAM bank 1
	;
	jp	6

load_data:	;' TODO
	ret
	