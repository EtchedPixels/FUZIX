;
;	Assembled after the patched base ROM. We do the start up and flip
;	into soft81 mode. This code is at 0x2000
;
		.module soft81boot
		.area	BOOT	(ABS)
		.org	0x1000
boot:
kscan:
	jr	bootgo		; Gets trampled by keyscan codes
sysvec:	jp	0x0000		; Patched by loader at offset 0x1002
	;	Fixed address so it's easy to patch the base ROM accordingly
	jp	exitfunc
	
bootgo:
	pop	de
	pop	hl		; address
	pop	bc		; fd of .p if non zero
	push	bc
	push	hl
	push	de

	push	bc		; Save loader handle until we are in position

	; HL holds the offset we are running from as we've not yet
	; relocated. We need to be relocatable for this bit
	ld	bc,(0x0101)	; JP stub vector for syscall
	ld	de,#0x1003
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
	ld	bc,#0x1400-0x69
	ldir			; Copy the rest of the modified ROM
	jp	bootup
bootup:				; Into the correct mapping
	ld	hl,#0xffff
	ld	(kscan),hl	; So we don't see ghost keys on boot
	pop	bc
	ld	(loadfd),bc	; file handle of a .p file */
	ld	a,#0xC3
	ld	(0x0207),a	; LOAD becomes JP loader */
	ld	hl,#loader
	ld	(0x0208),hl
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
cleanboot:
	;
	;	All systems go - no need to clean up stack
	;
	ld	hl,#0x7FFF	; We patched over this with our exit vector
	ld	a,#0x3F		; page before RAM
	jp	0x0261		; into ROM memory check
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

loader:
	ld	(savesp),sp
	ld	sp,#altsp
	ld	bc,(loadfd)
	ld	a,b
	or	c
	jr	z, cleanboot
	;	Loading a .p file at 0x4000
	ld	hl,#16384	; length
	push	hl
	ld	hl,#0x4000	; pointer
	push	hl
	push	bc		; file handle
	push	af		; dummy
	ld	a,#7
	call	sysvec		; read(fd, 0x4009, 16375);
	jr	c,exitfunc1	; failed
	;	Restore stack, don't bother cleaning up the alt stack
	ld	sp,(savesp)
	jp	0x0283

loadfd:
	.word	0
savesp:
	.word	0
	.ds	256
altsp:


;
;	The other mods to the ROM are
;
;	0x0000	C3 05 20	; so 0 causes an exit
;
;	And the system vectors for NMI and IM1 (RST 38h) are not copied over
;
