#include "build/kernel.def"
#include "kernel816.def"

	.export __uget, __ugetc, __ugetw
	.export __uput, __uputc, __uputw, __uzero

	.65c816
	.a16
	.i16

;
;	ptr1 and tmp1 are reserved for map_* functions in 6502 but
;	are actually free here.
;
	.code

; user, dst, count(count in ax)
;
;	These use mvn which uses Y so we need interrupts off. We might want
;	to do some other kind of fast copy loop if possible or split this
;	into say 256 byte bursts with ints on/off between ?
;
__uget:
	php
	sei
	phy
	lda	U_DATA__U_PAGE
	sep	#0x20
	.a8
	sta	KERNEL_CODE+kpatch+1		; FIXME: f:KERNEL_CODE+
					; as we might be split I/D
	rep	#0x20
	.a16
	lda	4,y			; number
	beq	nomove
	ldx	0,y			; src
	pha
	lda	2,y			; dst
	tay
	pla
	phb
kpatch:
	mvn	#0,#KERNEL_BANK
	plb
nomove:
	ply
	lda	#0
	plp
	rts

__ugetc:
	ldx	0,y
	phb
	lda	U_DATA__U_PAGE
	pha
	plb
	sep	#0x20
	.a8
	lda	0,x
	rep	#0x20
	.a16
	plb
	iny
	iny
	rts

__ugetw:
	ldx	0,y
	phb
	lda	U_DATA__U_PAGE
	pha
	plb
	lda	0,x
	plb
	iny
	iny
	rts

__uput:
	php
	sei
	phy
	lda	U_DATA__U_PAGE
	sep	#0x20
	.a8
	sta	KERNEL_CODE+kpatch2+2		; FIXME: far
					; as we might be split I/D
	rep	#0x20
	.a16
	lda	4,y			; number
	beq	nomove
	ldx	0,y			; src
	pha
	lda	2,y			; dst
	tay
	pla
	phb
kpatch2:
	mvn	#KERNEL_BANK,#0
	plb
nomove:
	ply
	lda	#0
	plp
	jmp	__fnexit6

__uputc:
	ldx	2,y
	phb
	lda	U_DATA__U_PAGE
	pha
	lda	0,y
	plb
	.a8
	rep	#0x20
	sta	0,x
	.a16
	sep 	#0x20
	plb
	lda	#0
	iny
	iny
	iny
	iny
	rts

__uputw:
	ldx	2,y
	phb
	lda	U_DATA__U_PAGE
	pha
	lda	0,y
	plb
	sta	0,x
	plb
	lda	#0
	iny
	iny
	iny
	iny
	rts

__uzero:
	lda	U_DATA__U_PAGE
	sta	KERNEL_CODE+uzero_patch+1	; FIXME far
	sta	KERNEL_CODE+uzero_patch+2

	; Clear lead byte in user space
	ldx	0,y
	phb
	pha
	plb
	lda	#0
	sep	#0x20
	.a8
	sta	0,x
	rep	#0x20
	.a16
	plb

	; Set up an overlapping block move
	ldx	0,y
	lda	2,y
	beq	nowork
	dec	a
	beq	nowork

	phb	; save bank
	php	; save flags
	sei	; ints off
	phy	; save stack

	txy	; set y up as overlap ptr (x + 1)
	iny	; for destination
	; Use mvn to wipe the range required
	; The set up is worth it as most uzero() calls are big
	; ranges
uzero_patch:
	mvn	#0,#0
	ply	; get c stack back
	plp	; interrupts back as they were
nowork:
	plb	; bank back as it was
	lda	#0 ; all good
	rts
