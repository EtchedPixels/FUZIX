	.include "platform/kernel.def"
        .include "kernel02.def"
	.include "platform/zeropage.inc"

		.export __uget, __ugetc, __ugetw, __ugets
		.export __uput, __uputc, __uputw, __uzero

		.import map_kernel, map_process_always
		.import popax
		.importzp ptr1, tmp1
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

__uputc:	sta ptr1
		stx ptr1+1
		jsr map_process_always
		jsr popax
		ldy #0
		sta (ptr1),y
		jmp map_kernel

__uputw:	sta ptr1
		stx ptr1+1
		jsr map_process_always
		jsr popax
		ldy #0
		sta (ptr1),y
		txa
		iny
		sta (ptr1),y
		jmp map_kernel

__uzero:	sta tmp1
		stx tmp1+1
		jsr map_process_always
		jsr popax		; ax is now the usermode address
		sta ptr1
		stx ptr1+1

		ldy #0
		tya

		ldx tmp1+1		; more than 256 bytes
		beq __uzero_tail	; no - just do dribbles
__uzero_blk:
		sta (ptr1),y
		iny
		bne __uzero_blk
		inc ptr1+1		; next 256 bytes
		dex			; are we done with whole blocks ?
		bne __uzero_blk

__uzero_tail:
		cpy tmp1
		beq __uzero_done
		sta (ptr1),y
		iny
		bne __uzero_tail
__uzero_done:	rts
