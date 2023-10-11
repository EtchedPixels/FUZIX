#include "../build/kernel.def"
#include "kernel816.def"

	.export __uget
	.export __ugetc
	.export __ugetw
	.export __uput
	.export	__uputc
	.export __uputw
	.export __uzero

	.65c816
	.a16
	.i16

;
;	Unoptimized reference code for now
;
	.code

; user, dst, count(count in ax)
;
__uget:
	tyx		; need in x for dec
	lda	0,y
	sta	@tmp
	lda	2,y
	sta	@tmp2
	lda	4,y
	beq	ugetdone
	and	#1
	beq	ugetl
	jsr	map_proc_always
	sep	#0x20
	.a8
	lda	(@tmp)
	rep	#0x20
	.a16
	jsr	map_kernel
	sep	#0x20
	.a8
	sta	(@tmp2)
	rep	#0x20
	.a16
	dec	4,x
	inc	@tmp
	inc	@tmp2
	lsr	4,x
	beq	ugetdone
	; We have a byte to do first
ugetl:
	jsr	map_proc_always
	lda	(@tmp)
	jsr	map_kernel
	sta	(@tmp)
	inc	@tmp
	inc	@tmp
	inc	@tmp2
	inc	@tmp2
	dec	4,x
	bne	ugetl
ugetdone:
	lda	#0
	iny
	iny
	iny
	iny
	iny
	iny
	rts

__ugetc:
	ldx	0,y
	jsr	map_proc_always
	sep	#0x20
	.a8
	lda	0,x
	rep	#0x20
	.a16
	jsr	map_kernel
	rts

__ugetw:
	ldx	0,y
	jsr	map_proc_always
	lda	0,x
	jsr	map_kernel
	rts

__uput:
	tyx		; need in x for dec
	lda	0,y
	sta	@tmp
	lda	2,y
	sta	@tmp2
	lda	4,y
	beq	uputdone
	and	#1
	beq	uputl
	sep	#0x20
	.a8
	lda	(@tmp)
	rep	#0x20
	.a16
	jsr	map_proc_always
	sep	#0x20
	.a8
	sta	(@tmp2)
	rep	#0x20
	.a16
	dec	4,x
	inc	@tmp
	inc	@tmp2
	lsr	4,x
	beq	uputdone
	; We have a byte to do first
uputl:
	jsr	map_kernel
	lda	(@tmp)
	jsr	map_proc_always
	sta	(@tmp)
	inc	@tmp
	inc	@tmp
	inc	@tmp2
	inc	@tmp2
	dec	4,x
	bne	uputl
uputdone:
	lda	#0
	iny
	iny
	iny
	iny
	iny
	iny
	rts

__uputc:
	ldx	2,y
	jsr	map_proc_always
	lda	0,y		; kstack is in common so this is ok
	sep	#0x20
	.a8
	sta	0,x
	rep	#0x20
	.a16
	jsr	map_kernel
	iny
	iny
	iny
	iny
	rts

__uputw:
	ldx	2,y
	jsr	map_proc_always
	lda	0,y		; kstack is in common so this is ok
	sta	0,x
	jsr	map_kernel
	iny
	iny
	iny
	iny
	rts

__uzero:
	ldx	0,y
	lda	2,y
	sta	@tmp
	beq	uzdone
	and	#1
	beq	uzloop
	jsr	map_proc_always
	sep	#0x20
	.a8
	stz	0,x
	rep	#0x20
	.a16
	inx
	lsr	@tmp
	beq	uzdone
uzloop:	stz	0,x
	inx
	inx
	dec	@tmp
	bne	uzloop
uzdone:	iny
	iny
	iny
	iny
	rts

