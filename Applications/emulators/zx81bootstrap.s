;
;	Patch and load ROM
;
	.module soft81boot
	.area	_CODE

	.globl _bootstrap

_bootstrap:			; We must be relocatable, HL is base
				; PC is at HL - 0x2000

	ld	bc,(0x0101)	; JP stub vector for syscall
	ld	de,#0x2003
	ex	de,hl
	add	hl,de		; DE is our base, HL is now the patch spot
	ld	(hl),c
	inc	hl
	ld	(hl),b
	ex	de,hl		; HL is now our base again

	ld	de,#0
	ld	bc,#0x38
	ldir			; Copy 0x38 bytes including JP stub
	inc	hl
	inc	hl
	inc	hl
	inc	de		; Skip interrupt vector
	inc	de
	inc	de		; 0x38-0x3A
	ld	bc,#0x66-0x3A
	ldir
	inc	hl		; Skip NMI vector
	inc	hl
	inc	hl
	inc	de
	inc	de
	inc	de
	ld	hl,#0x2400-0x69
	ldir			; Copy the rest of the modified ROM
	jp	0x2000		; Jump to the boot code in the ROM image

