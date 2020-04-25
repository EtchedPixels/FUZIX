;
; 1998-06-06, Ullrich von Bassewitz
; 2015-09-11, Greg King
;
; void __fastcall__ longjmp (jmp_buf buf, int retval);
;
; Modified for the Fuzix 65C816 setup
;

        .export         _longjmp
        .import         popax
        .importzp       sp, ptr1, ptr2
	.importzp	tmp1,tmp2

	.p816
	.a8
	.i8

_longjmp:
        sta     ptr2            ; Save retval
        stx     ptr2+1
        ora     ptr2+1          ; Check for 0
        bne     @L1
        inc     ptr2            ; 0 is illegal, according to the standard ...
                                ; ... and, must be replaced by 1
@L1:    jsr     popax           ; get buf
        sta     ptr1
        stx     ptr1+1
        ldy     #0

; Get the old parameter stack

        lda     (ptr1),y
        iny
        sta     sp
        lda     (ptr1),y
        iny
        sta     sp+1

;
; Detection code based on an idea by Jody Bruchon
;
; Party time we have three cases
;
; 6502/65C02/65WD02 etc		- S is 8bit
; 65C816 in compat mode		- S is 8bit
; 65C816 in native mode		- S is 16bit and we must merge the saved low
;				  8 with the current high 8 with IRQs off
;				  due to the sucky CPU design
;
	lda #$00
	inc			; 65C02 or later will add one
	cmp #$01
	bmi is_8bit		; 6502so skip
;
;	We can now safely play with xba to see if it's an 816.
; 
	xba			; 65c02 the xba's do nothing		
	dec			; so a goes to 0
	xba			; while 65C816 keeps it as one
	cmp #$01
	bmi is_8bit
;
;	16bit mode. Please ensure your barf bucket is to hand
;
;	If we are in compat mode then the rep/sep do nothing, we load
;	the 8bit stack pointer into X and then A, we rewrite it with the
;	8bit new S value and we write it back into S
;
;	If we are in native mode the rep/sep do stuff so we load a 16bit
;	S into X and then A, we overwrite the low 8bits with the saved S
;	and we stick the lot back together and put it into X and then S
;
	sei			; Interrupts off
	rep #$30
	.a16
	.i16
	tsx			; 16 bit stack pointer into A
	txa
	sep #$20
	.a8
        lda     (ptr1),y	; Restore low 8bits of SP only
	rep #$20
	.a16

        iny			; Move down struct

	tax			; Restore stack pointer 16bits
	txs

	sep #$30
	.a8
	.i8

	cli			; Interrupts back on
	bra pop_out

;
; 8bit is nice and simple
;
is_8bit:	
; Get the old stack pointer

        lda     (ptr1),y
        iny
        tax

        txs

pop_out:

; Get the return address and push it on the stack

        lda     (ptr1),y
        iny
        pha
        lda     (ptr1),y
        pha

; Load the return value and return to the caller

        lda     ptr2
        ldx     ptr2+1
        rts
