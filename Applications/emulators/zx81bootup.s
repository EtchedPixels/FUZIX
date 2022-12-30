;
;	Assembled after the patched base ROM. We do the start up and flip
;	into soft81 mode. This code is at 0x2000
;
		.module soft81boot
		.area	BOOT	(ABS)
		.org	0x2000
bootup:
kscan:
	jr	bootgo		; Gets trampled by keyscan codes
sysvec:	jp	0x0000		; Patched by loader at offset 0x2002
	;	Fixed address so it's easy to patch the base ROM accordingly
	jp	exitfunc
	
bootgo:
	ld	hl,#0xffff
	ld	(kscan),hl	; So we don't see ghost keys on boot
	ld	hl,#0x0001
	push	hl
	ld	hl,#0712
	push	hl
	ld	hl,#0x0000
	push	hl
	ld	a,#29		; ioctl
	call	sysvec		; bootstrap helper saved syscall vector and
				; patched it in for us
	jr	c,exitfunc1	; 
	; All systems go
	
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
