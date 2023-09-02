	.globl _uputc
	.globl _uputw
	.globl _ugetc
	.globl _ugetw
	.globl _uzero
	.globl _uget
	.globl _uput

	.section .common

_uputc:
	tsx
	ldx 2,x
	jsr map_proc_always
	stab 0,x
	jmp map_kernel

_uputw:
	tsx
	ldx 2,x
	jsr map_proc_always
	std 0,x
	jmp map_kernel

_ugetc:
	xgdx
	jsr map_proc_always
	ldab 0,x
	clra
	jmp map_kernel

_ugetw:
	xgdx
	jsr map_proc_always
	ldd 0,x
	jmp map_kernel

_uzero:
	tsx
	ldx 2,x			; size
	xgdx			; swap over as we want pointer in X
	jsr map_proc_always
	; Optimize, unroll etc ?
uzerolp:
	clr 0,x
	inx
	subd #1
	bne uzerolp
	jmp map_kernel

_uget:	
	; source D dest 3 len 5
	std __end
	xgdy		; Y is now source
	tsx
	ldd 4,x		; len
	beq nowork
	addd __end
	std __end	; stop marker
	ldx 2,x		; to
ugetl:
	jsr map_proc_always
	ldab ,y
	iny
	jsr map_kernel
	stab ,x
	inx
	cpy __end
	bne ugetl
nowork:
	clrb
	clra
	rts

_uput:	
	; source D dest 3 len 5
	std __end
	xgdy		; Y is now source
	tsx
	ldd 4,x		; len
	beq nowork
	addd __end
	std __end	; stop marker
	ldx 2,x		; to
uputl:
	ldab ,y
	iny
	jsr map_proc_always
	stab ,x
	inx
	jsr map_kernel
	cpy __end
	bne uputl
	clrb
	clra
	rts


	.section .data

;
;	We should be ok not saving these on IRQ as we don't allow IRQs to
;	poke user space. TODO: move some of these to ZP
;
__end:
	.word 0
