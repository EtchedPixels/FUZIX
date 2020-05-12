	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.common

__uputc:
	tsx
	ldab 4,x
	ldx 3,x
	jsr map_process_always
	stab 0,x
	jsr map_kernel
	jmp ret2

__uputw:
	tsx
	ldaa 5,x
	ldab 6,x
	ldx 3,x
	jsr map_process_always
	staa 0,x
	stab 1,x
	jsr map_kernel
	jmp ret2

__ugetc:
	tsx
	ldx 3,x
	jsr map_process_always
	ldab 0,x
	clra
	jmp map_kernel

__ugetw:
	tsx
	ldx 3,x
	jsr map_process_always
	ldab 0,x
	ldaa 1,x
	jmp map_kernel

__uzero:
	tsx
	ldaa 3,x
	ldab 4,x
	ldx 5,x
	jsr map_process_always
	; Optimize, unroll etc ?
uzerolp:
	clr 0,x
	inx
	subb #1
	sbca #0
	bne uzerolp
	jmp map_kernel

__uget:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldaa 3,x	; len
	ldab 4,x
	staa @tmp
	stab @tmp
	ldaa 5,x	; to
	ldab 6,x
	staa @to
	stab @to+1
	ldx 7,x		; from
	stx @from
ugetl:
	ldx @from
	jsr map_process_always
	ldab ,x
	inx
	jsr map_kernel
	stx @from
	ldx @to
	stab ,x
	inx
	stx @to
	ldab @tmp
	ldaa @tmp+1
	subb #1
	sbca #0
	staa @tmp
	stab @tmp+1
	bne ugetl
	clrb
	clra
	jmp ret6

__uput:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldaa 3,x	; len
	ldab 4,x
	staa @tmp
	stab @tmp+1
	ldaa 5,x	; to
	ldab 6,x
	staa @to
	stab @to+1
	ldx 7,x		; from
	stx @from
uputl:
	ldx @from
	ldab ,x
	inx
	stx @from
	ldx @to
	jsr map_process_always
	stab ,x
	jsr map_kernel
	inx
	stx @to
	ldaa @tmp
	ldab @tmp+1
	subb #1
	sbca #0
	staa @tmp
	stab @tmp+1
	bne uputl
	clrb
	clra
	jmp ret6

	.zp

;
;	We should be ok not saving these on IRQ as we don't allow IRQs to
;	poke user space
;
to:
	.word 0
from:
	.word 0
