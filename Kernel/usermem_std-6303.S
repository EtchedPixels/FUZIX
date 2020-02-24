	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.code

__uputc:
	tsx
	ldab 4,x
	ldx 5,x
	jsr map_process_always
	stab 0,x
	jmp map_kernel

__uputw:
	tsx
	ldd 3,x
	ldx 5,x
	jsr map_process_always
	stab 0,x
	jmp map_kernel

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
	ldd 0,x
	jmp map_kernel

__uzero:
	tsx
	ldd 5,x
	ldx 3,x
	jsr map_process_always
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
	ldd 7,x		; len
	std tmp
	ldd 5,x		; to
	std to
	ldx 3,x		; from
	stx from
ugetl:
	ldx from
	jsr map_process_always
	ldab ,x
	inx
	jsr map_kernel
	stx from
	ldx to
	stab ,x
	inx
	stx to
	ldd tmp
	subd #1
	std tmp
	bne ugetl
	clrb
	clra
	rts

__uput:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	tsx
	ldd 7,x		; len
	std tmp
	ldd 5,x		; to
	std to
	ldx 3,x		; from
	stx from
uputl:
	ldx from
	ldab ,x
	inx
	stx from
	ldx to
	jsr map_process_always
	stab ,x
	jsr map_kernel
	inx
	stx to
	ldd tmp
	subd #1
	std tmp
	bne uputl
	clrb
	clra
	rts

