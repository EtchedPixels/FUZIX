;
;	We are loaded from the boot block by the firmware and are somewhere
;	in the original mapping 0 low, 1 high at E000. Stack is at FExx
;	somewhere.
;

		.code
	.byte 'Z'
	.byte 8

start:
;
;	Fix up the MMU mappings so we have RAM low as well
;
	di			; We are about to map out the irqvecs
	ld	r14,#0xFF
	ld	r15,r14
	ld	r13,#0x11
	and	2,#0xBF		; I/O on for upper data space
	lde	@rr14,r13
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
	ld	r14,#0xFF
	ld	r15,r14
	ld	r13,#0x31
	and	2,#0xBF
	lde	@rr14,r13
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
	ld	r14,#0xFF
	ld	r15,#0xFF
	and	2,#0xBF
	ld	r13,#0x33	; Set data
	lde	@rr14,r13
	dec	r15
	ld	r13,#0x11	; Set code
	lde	@rr14,r13
	or	2,#0x10
	;
	;	We now have code mapping of
	;	0000-DFFF : code	RAM bank 0
	;	E000-FFFF : loader etc	RAM bank 0
	;	Data mapping of
	;	0000-7FFF : data	RAM bank 1
	;	E000-FFFF : data	RAM bank 1
	;
	jmp	6
	