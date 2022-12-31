;
;	Assembled after the patched base ROM. We do the start up and flip
;	into soft81 mode. This code is at 0x2000
;
		.module soft81boot
		.area	BOOT	(ABS)
		.org	0x2000
boot:
kscan:
	jr	bootgo		; Gets trampled by keyscan codes
sysvec:	jp	0x0000		; Patched by loader at offset 0x2002
	;	Fixed address so it's easy to patch the base ROM accordingly
	jp	exitfunc
	
bootgo:
	pop	de
	pop	hl
	push	hl
	push	de
	; HL holds the offset we are running from as we've not yet
	; relocated. We need to be relocatable for this bit
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
	ld	bc,#0x2400-0x69
	ldir			; Copy the rest of the modified ROM
	jp	bootup
bootup:				; Into the correct mapping
	ld	hl,#0xffff
	ld	(kscan),hl	; So we don't see ghost keys on boot
	ld	hl,#0x0001
	push	hl
	ld	hl,#0x1CA	; ZX81 mode on 0712 ioctl
	push	hl
	ld	hl,#0x0000
	push	hl
	push	af		; dummy as we are not going via the usual path
	ld	a,#29		; ioctl
	call	sysvec		; bootstrap helper saved syscall vector and
				; patched it in for us
	jr	c,exitfunc1	; 
	;
	;	All systems go - no need to clean up stack
	;
	ld	bc,#0x7FFF	; We patched over this with our exit vector
	jp	0x03CB		; into ROM memory check
exitfunc:
	ld	hl,#0x0000
exit2:
	inc	hl
	push	hl
	xor	a
	call	sysvec		; Will not return
	di
	halt			; We hope !
exitfunc1:
	ld	hl,#0x0001
	jr	exit2

;
;	The other mods to the ROM are
;
;	0x0000	C3 05 20	; so 0 causes an exit
;	0x027C  C9		; avoid running display bits
;	0x02BB	21 00 20 C9	; may change to a lookup and char table ?
;
;	And the system vectors for NMI and IM1 (RST 38h) are not copied over
;
