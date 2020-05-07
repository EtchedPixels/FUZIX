	.globl _uputc
	.globl _uputw
	.globl _ugetc
	.globl _ugetw
	.globl _uzero
	.globl _uget
	.globl _uput

	.section common

_uputc:
	tsx
	ldab 4,x
	ldx 3,x
	jsr map_process_always
	stab 0,x
	jmp map_kernel

_uputw:
	tsx
	ldd 5,x
	ldx 3,x
	jsr map_process_always
	std 0,x
	jmp map_kernel

_ugetc:
	tsx
	ldx 3,x
	jsr map_process_always
	ldab 0,x
	clra
	jmp map_kernel

_ugetw:
	tsx
	ldx 3,x
	jsr map_process_always
	ldd 0,x
	jmp map_kernel

_uzero:
	tsx
	ldd 3,x
	ldx 5,x
	jsr map_process_always
	; Optimize, unroll etc ?
uzerolp:
	clr 0,x
	inx
	subd #1
	bne uzerolp
	jmp map_kernel

_uget:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldd 3,x		; len
	std __tmp
	ldd 5,x		; to
	std __to
	ldx 7,x		; from
	stx __from
ugetl:
	ldx __from
	jsr map_process_always
	ldab ,x
	inx
	jsr map_kernel
	stx __from
	ldx __to
	stab ,x
	inx
	stx __to
	ldd __tmp
	subd #1
	std __tmp
	bne ugetl
	clrb
	clra
	rts

_uput:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldd 3,x		; len
	std __tmp
	ldd 5,x		; to
	std __to
	ldx 7,x		; from
	stx __from
uputl:
	ldx __from
	ldab ,x
	inx
	stx __from
	ldx __to
	jsr map_process_always
	stab ,x
	jsr map_kernel
	inx
	stx __to
	ldd __tmp
	subd #1
	std __tmp
	bne uputl
	clrb
	clra
	rts

	.section data

;
;	We should be ok not saving these on IRQ as we don't allow IRQs to
;	poke user space. TODO: move some of these to ZP
;
__to:
	.word 0
__from:
	.word 0
__tmp:
	.word 0
