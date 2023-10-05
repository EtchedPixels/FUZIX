;
;	Driver routines for firmware interfaces
;
            .include "kernel.def"
            .include "../../cpu-6502/kernel02.def"
	    .include "zeropage.inc"

	.export _pascal_op
	.export _pascal_cmd
	.export _dos_op
;
;	Issue pascal command X to slot A. Note that we don't check for valid
;	firmware here. That was done earlier. If it's not valid we jump to
;	fishkill.
;
_pascal_op:
	stx cmdbyte		; patch the operation data following the jsr
	clc
	adc #$C0		; Turn slot into slot address high
	sta ptr1+1		; xxFF holds the index to the DOS entry
	lda #255
	sta ptr1
	ldy #0
	lda (ptr1),y		; index
	clc
	adc #3			; and 3 beyond that (next JMP) is the pascal
	sta ptr1		; one.
	jsr goptr1		; Fake a jsr (ptr1)
cmdbyte:
	.byte $00
	.word _pascal_cmd+1
	rts

goptr1:	jmp (ptr1)

;
;	Issue a generic ProDOS command block
;
_dos_op:
	clc
	adc #$C0
	sta ptr1+1
	lda #255
	sta ptr1
	ldy #0
	lda (ptr1),y
	sta ptr1
	jmp (ptr1)

;
;	The caller loads up to 9 bytes of data into this buffer
;
	.bss
_pascal_cmd:	.res 9


