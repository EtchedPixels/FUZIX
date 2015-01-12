	.include "platform/kernel.def"
        .include "kernel02.def"
	.include "platform/zeropage.inc"

		.export __uget, __ugetc, __ugetw, __ugets
		.export __uput, __uputc, __uputw, __uzero

		.import map_kernel, map_process_always

;
;	TO DO: Possibly easier to write them in C with just the _c helpers
;	but for speed would be best the block ones are asm.
;
;
;	These methods are intended for a 6502. The 6509 will need something
;	quite different (and in truth rather more elegant!)
;
		.segment "COMMONMEM"

__uget:		rts

__ugetc:	sta ptr1
		stx ptr1+1
		jsr map_process_always
		ldy #0
		lda (ptr1),y
		jmp map_kernel

__ugetw:	sta ptr1
		stx ptr1+1
		jsr map_process_always
		ldy #1
		lda (ptr1),y
		tax
		dey
		lda (ptr1),y
		jmp map_kernel

__ugets:	rts
__uput:		rts
__uputc:	rts
__uputw:	rts
__uzero:	rts
