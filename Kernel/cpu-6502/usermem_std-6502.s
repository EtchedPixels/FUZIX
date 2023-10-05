	.include "../build/kernel.def"
        .include "kernel02.def"
	.include "../build/zeropage.inc"

		.export __uget, __ugetc, __ugetw
		.export __uput, __uputc, __uputw, __uzero

		.import map_kernel, map_proc_always
		.import outxa, popax
		.importzp ptr2, tmp2
;
;	These are intended as reference implementations to get a platform
;	booting. There are several things to consider once you are running
;
;	If your common memory is in RAM you could make uget/uput/uzero all
;	a single self modifying routine. We don't do it here as it's
;	reference code
;
;	If you have flexibility in the banking then odds-on it will be much
;	faster to map a bank somewhere and copy via the mapping while taking
;	care to deal with moving between banks
;
;	If you don't have the flexibility then depending upon your map
;	routines it is likely to be faster to double buffer.
;
;	These methods are intended for a 6502. The 6509 has far copiers
;	using the (foo),y addressing. In those cases you can remove the map_
;	calls but need to deal with the lack of (foo),x to do bank to bank.
;
;	ptr1 and tmp1 are reserved for map_* functions
;
;	This code relies upon the fact that the ZP is not switched when
;	switching between user and kernel for the same process.
;
;
		.segment "COMMONMEM"

; user, dst, count(count in ax)
;
;	Decidedly unoptimised (even the 6502 could manage a word a switch)
;
__uget:		sta tmp2
		stx tmp2+1		; save the count
		jsr popax		; pop the destination
		sta ptr2		; (ptr2) is our target
		stx ptr2+1
		jsr popax		; (ptr2) is our source
		sta ptr3
		stx ptr3+1

		ldy #0			; counter

		ldx tmp2+1		; how many 256 byte blocks
		beq __uget_tail		; if none skip to the tail

__uget_blk:
		jsr map_proc_always	; map the user process in
		lda (ptr3), y		; get a byte of user data
		jsr map_kernel		; map the kernel back in
		sta (ptr2), y		; save it to the kernel buffer
		iny			; move on one
		bne __uget_blk		; not finished a block ?
		inc ptr2+1		; move src ptr 256 bytes on
		inc ptr3+1		; move dst ptr the same
		dex			; one less block to do
		bne __uget_blk		; out of blocks ?

__uget_tail:	cpy tmp2		; finished ?
		beq __uget_done

		jsr map_proc_always	; map the user process
		lda (ptr3),y		; get a byte of user data
		jsr map_kernel		; map the kernel back in
		sta (ptr2),y		; save it to the kernel buffer
		iny			; move on
		bne __uget_tail		; always taken (y will be non zero)

__uget_done:
		lda #0
		tax
		rts

__ugetc:	sta ptr2
		stx ptr2+1
__uget_ptr2:
		jsr map_proc_always
		ldy #0
		lda (ptr2),y
		jmp map_kernel

__ugetw:	sta ptr2
		stx ptr2+1
		jsr map_proc_always
		ldy #1
		lda (ptr2),y
		tax
		dey
		lda (ptr2),y
		jmp map_kernel


__uput:		sta tmp2
		stx tmp2+1
		jsr popax	; dest
		sta ptr2
		stx ptr2+1
		jsr popax	; source
		sta ptr3
		stx ptr3+1

		ldy #0

		ldx tmp2+1
		beq __uput_tail
__uput_blk:
		jsr map_kernel
		lda (ptr3), y
		jsr map_proc_always
		sta (ptr2), y
		iny
		bne __uput_blk
		inc ptr2+1
		inc ptr3+1
		dex
		bne __uput_blk

__uput_tail:	cpy tmp2
		beq __uput_done
		jsr map_kernel
		lda (ptr3),y
		jsr map_proc_always
		sta (ptr2),y
		iny
		bne __uput_tail

__uput_done:
		jsr map_kernel
		lda #0
		tax
		rts

__uputc:	sta ptr2
		stx ptr2+1
		jsr popax
		jsr map_proc_always
		ldy #0
		sta (ptr2),y
		jmp map_kernel

__uputw:	sta ptr2
		stx ptr2+1
		jsr popax
		jsr map_proc_always
		ldy #0
		sta (ptr2),y
		txa
		iny
		sta (ptr2),y
		jmp map_kernel

__uzero:	sta tmp2
		stx tmp2+1
		jsr popax		; ax is now the usermode address
		sta ptr2
		stx ptr2+1

		; Our C stack vanishes at this point
		jsr map_proc_always

		ldy #0
		tya

		ldx tmp2+1		; more than 256 bytes
		beq __uzero_tail	; no - just do dribbles
__uzero_blk:
		sta (ptr2),y
		iny
		bne __uzero_blk
		inc ptr2+1		; next 256 bytes
		dex			; are we done with whole blocks ?
		bne __uzero_blk

__uzero_tail:
		cpy tmp2
		beq __uzero_done
		sta (ptr2),y
		iny
		bne __uzero_tail
__uzero_done:	jmp map_kernel


