	.include "platform/kernel.def"
        .include "kernel816.def"
	.include "platform/zeropage.inc"

	.export __uget, __ugetc, __ugetw, __ugets
	.export __uput, __uputc, __uputw, __uzero

	.import outxa, popax
	.importzp ptr2, tmp2

	.p816
	.a8
	.i8

;
;	ptr1 and tmp1 are reserved for map_* functions in 6502 but
;	are actually free here.
;
	.code

; user, dst, count(count in ax)
;
;	Compiler glue is not pretty - might be worth having some optimized
;	16bit aware stack handlers
;
__uget:	sta	ptr1
	stx	ptr1+1			; save the count
	jsr	popax			; pop the destination
	sta	ptr2			; (ptr2) is our target
	stx	ptr2+1
	jsr	popax			; (ptr3) is our source
	sta	ptr3
	stx	ptr3+1
	lda	U_DATA__U_PAGE
	sta	ugetpatch+2
	phb
	.i16
	.a16
	rep	#$30
	ldx	ptr3			; source
	ldy	ptr2			; destination
	lda	ptr1
	beq	ug_nomov		; 0 means 64K!
	dec				; need 1 less than size
ugetpatch:
	mvn	0,KERNEL_FAR
ug_nomov:
	.i8
	.a8
	sep	#$30
	plb
	lda	#0
	tax
	rts

;
;	This could be done more nicely using .i16 and wants optimizing
;
__ugets:
	sta	tmp2
	stx	tmp2+1			; save the count
	jsr	popax			; pop the destination
	sta	ptr2			; (ptr2) is our target
	stx	ptr2+1
	jsr	popax			; (ptr2) is our source
	sta	ptr3
	stx	ptr3+1

	ldy	#0			; counter

	ldx	tmp2+1			; how many 256 byte blocks
	beq	__ugets_tail		; if none skip to the tail

__ugets_blk:
	phb
	lda	U_DATA__U_PAGE		; switch to user bank read
	pha
	plb
	lda	(ptr3), y		; get a byte of user data
	beq	__ugets_end
	plb				; back to kernel
	sta	(ptr2), y		; save it to the kernel buffer
	iny				; move on one
	bne	__ugets_blk		; not finished a block ?
	inc	ptr3+1			; move src ptr 256 bytes on
	inc	ptr2+1			; move dst ptr the same
	dex				; one less block to do
	bne	__ugets_blk		; out of blocks ?

__ugets_tail:
	cpy	tmp2			; finished ?
	beq	__ugets_bad

	phb
	lda	U_DATA__U_PAGE
	pha
	plb
	lda	(ptr3),y		; get a byte of user data
	beq	__ugets_end
	plb
	sta	(ptr2),y		; save it to the kernel buffer
	iny				; move on
	bne	__ugets_tail		; always taken (y will be non zero)

__ugets_bad:
	dey
	lda	#0
	sta	(ptr2), y		; terminate kernel buffer
	lda	#$FF			; string too large
	tax				; return $FFFF
	rts

__ugets_end:
	plb
	lda	#0
	sta	(ptr2), y
	tax
	rts

__ugetc:
	sta	ptr2
	stx	ptr2+1
	phb
	lda	U_DATA__U_PAGE
	pha
	plb
	lda	(ptr2)
	plb
	rts

__ugetw:
	sta	ptr2
	stx	ptr2+1
	phb
	lda	U_DATA__U_PAGE
	pha
	plb
	ldy	#1
	lda	(ptr2),y
	tax
	lda	(ptr2)
	plb
	rts


__uput:
	sta	ptr1
	stx	ptr1+1
	jsr	popax			; dest
	sta	ptr2
	stx	ptr2+1
	jsr	popax			; source
	sta	ptr3
	stx	ptr3+1
	lda	U_DATA__U_PAGE
	sta	uputpatch+1
	phb
	.i16
	.a16
	rep	#$30
	ldx	ptr3			; source
	ldy	ptr2			; destination
	lda	ptr1
	beq	up_nomov		; 0 means 64K!
	dec				; need 1 less than size
uputpatch:
	mvn	KERNEL_FAR,0
up_nomov:
	.i8
	.a8
	sep	#$30
	plb
	lda	#0
	tax
	rts

__uputc:
	sta	ptr2
	stx	ptr2+1
	jsr	popax
	phb
	pha
	lda	U_DATA__U_PAGE
	pha
	plb
	pla
	sta	(ptr2)
	plb
	lda	#0
	tax
	rts

__uputw:
	sta	ptr2
	stx	ptr2+1
	jsr	popax
	phb
	pha
	lda	U_DATA__U_PAGE
	pha
	plb
	pla
	sta	(ptr2)
	txa
	ldy	#1
	sta	(ptr2),y
	plb
	lda	#0
	tax
	rts

__uzero:
	sta	tmp2
	stx	tmp2+1
	jsr	popax			; ax is now the usermode address
	sta	ptr2
	stx	ptr2+1

	lda	U_DATA__U_PAGE
	sta	uzero_patch+1
	sta	uzero_patch+2

	; Clear lead byte in user space
	phb
	pha
	plb
	lda	#0
	sta	(ptr2)
	plb

	phb
	.i16
	.a16
	rep	#$30
	ldx	ptr2			; copy from x to x+1 moving the 0 up
	ldy	ptr2
	iny
	lda	tmp2			; no bytes means we are done
	beq	nozero
	dec				; 1 byte means our clz did it
	beq	nozero
	; Use mvn to wipe the range required
	; The set up is worth it as most uzero() calls are big
	; ranges
uzero_patch:
	mvn	0,0
nozero:
	plb
	sep	#$30
	.i8
	.a8
	lda	#0
	tax
	rts
