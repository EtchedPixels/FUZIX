	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.setcpu 6803

	.common

__uputc:
	tsx
	ldab 5,x
	ldx 2,x
	jsr map_proc_always
	stab 0,x
	jmp map_kernel

__uputw:
	tsx
	ldd 4,x
	ldx 2,x
	jsr map_proc_always
	std 0,x
	jmp map_kernel

__ugetc:
	tsx
	ldx 2,x
	jsr map_proc_always
	ldab 0,x
	clra
	jmp map_kernel

__ugetw:
	tsx
	ldx 2,x
	jsr map_proc_always
	ldd 0,x
	jmp map_kernel

__uzero:
	tsx
	ldd 2,x
	ldx 4,x
	jsr map_proc_always
	; Optimize, unroll etc ?
uzerolp:
	clr 0,x
	inx
	subd #1
	bne uzerolp
	jmp map_kernel

__uget:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldd 2,x		; len
	std @tmp
	ldd 4,x		; to
	std @to
	ldx 6,x		; from
	stx @from
ugetl:
	ldx @from
	jsr map_proc_always
	ldab ,x
	inx
	jsr map_kernel
	stx @from
	ldx @to
	stab ,x
	inx
	stx @to
	ldd @tmp
	subd #1
	std @tmp
	bne ugetl
	clrb
	clra
	rts

__uput:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldd 2,x		; len
	std @tmp
	ldd 4,x		; to
	std @to
	ldx 6,x		; from
	stx @from
uputl:
	ldx @from
	ldab ,x
	inx
	stx @from
	ldx @to
	jsr map_proc_always
	stab ,x
	jsr map_kernel
	inx
	stx @to
	ldd @tmp
	subd #1
	std @tmp
	bne uputl
	clrb
	clra
	rts

	.zp

;
;	We should be ok not saving these on IRQ as we don't allow IRQs to
;	poke user space
;
to:
	.word 0
from:
	.word 0
