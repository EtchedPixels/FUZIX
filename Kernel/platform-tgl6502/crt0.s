        	; exported symbols
	        .export start
		.export copycommon

		; imported symbols
		.import init_early
		.import init_hardware
		.import _fuzix_main
		.import kstack_top
		.import vector
		.import nmi_handler

		.import  __BSS_RUN__, __BSS_SIZE__
		.import	 __DATA_LOAD__, __DATA_RUN__, __DATA_SIZE__
		.import	 __COMMONMEM_LOAD__, __COMMONMEM_RUN__, __COMMONMEM_SIZE__
		.import	 __STUBS_LOAD__, __STUBS_RUN__, __STUBS_SIZE__
		.importzp	ptr1, ptr2, tmp1

	        ; startup code @0
	        .code
		.include "zeropage.inc"

;
;	We are entered in ROM 8K bank $40, at virtual address $C000
;
		lda #'F'
		sta $FF03		; signal our arrival

		sei			; interrupts off
		cld			; decimal off
		ldx #$FF
		txs			; Stack (6502 not C)

;
;	We are in the wrong place at the wrong time
;
;	Map our page at $4000 as well
;
		ldx #$40		; our first 8K
		stx $FF8C		; at ROM 0x4000
;
;	And land in the right place...
;
		jmp start
start:					; Map ROM at 0x4000-0xFFFF
		lda #'u'
		sta $FF03
		inx			; ROM bank 1 is a magic hack we
		inx			; must skip over
		stx $FF8D
		inx
		stx $FF8E
		inx
		stx $FF8F
		inx
		stx $FF90
		inx
		stx $FF91
		lda #$02
		sta $FF8A		; Common for init at 0x0000
		lda #$01		; Kernel data at 0x2000
		sta $FF8B

		lda #<kstack_top	; C stack
		sta sp
		lda #>kstack_top
		sta sp+1 

		lda #<__BSS_RUN__
		sta ptr1
		lda #>__BSS_RUN__
		sta ptr1+1

		lda #'z'
		sta $FF03

		lda #0
		tay
		ldx #>__BSS_SIZE__
		beq bss_wipe_tail
bss_wiper_1:	sta (ptr1),y
		iny
		bne bss_wiper_1
		inc ptr1+1
		dex
		bne bss_wiper_1

bss_wipe_tail:
		cpy #<__BSS_SIZE__
		beq gogogo
		sta (ptr1),y
		iny
		bne bss_wipe_tail
gogogo:
		lda #'i'
		sta $FF03
		; But not just yet on the tgl6502 - need to sort the ROM
		; to RAM copy of the data out
		;
		; This code comes from the TGL support library
		; Ullrich von Bassewitz 1998-12-07
		;
		; Give me LDIR any day of the week
	        lda     #<__DATA_LOAD__         ; Source pointer
	        sta     ptr1
	        lda     #>__DATA_LOAD__
	        sta     ptr1+1

	        lda     #<__DATA_RUN__          ; Target pointer
	        sta     ptr2
	        lda     #>__DATA_RUN__
	        sta     ptr2+1

	        ldx     #<~__DATA_SIZE__
	        lda     #>~__DATA_SIZE__        ; Use -(__DATASIZE__+1)
	        sta     tmp1

		jsr	copyloop

	        lda     #<__STUBS_LOAD__         ; Source pointer
	        sta     ptr1
	        lda     #>__STUBS_LOAD__
	        sta     ptr1+1

	        lda     #<__STUBS_RUN__          ; Target pointer
	        sta     ptr2
	        lda     #>__STUBS_RUN__
	        sta     ptr2+1

	        ldx     #<~__STUBS_SIZE__
	        lda     #>~__STUBS_SIZE__        ; Use -(__STUBS_SIZE__+1)
	        sta     tmp1

		jsr	copyloop



		jsr	copycommon

		lda #'x'
		sta $FF03

		jsr init_early
		lda #'.'
		sta $FF03
		jsr init_hardware
		lda #13
		sta $FF03
		lda #10
		sta $FF03
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop


copyloop:
	        ldy     #$00

; Copy loop

@L1:	        inx
    	        beq     @L3

@L2:    	lda     (ptr1),y
        	sta     (ptr2),y
        	iny
        	bne     @L1
        	inc     ptr1+1
        	inc     ptr2+1                  ; Bump pointers
        	bne     @L1                     ; Branch always (hopefully)

; Bump the high counter byte

@L3:    	inc     tmp1
        	bne     @L2
		rts

; Done


;
;	This may also used be useful at runtime so split it off as a helper
;	We can revisit and unsplit it if not.
;
copycommon:
		pha
		txa
		pha
	        lda     #<__COMMONMEM_LOAD__         ; Source pointer
	        sta     ptr1
	        lda     #>__COMMONMEM_LOAD__
	        sta     ptr1+1

	        lda     #<__COMMONMEM_RUN__          ; Target pointer
	        sta     ptr2
	        lda     #>__COMMONMEM_RUN__
	        sta     ptr2+1

	        ldx     #<~__COMMONMEM_SIZE__
	        lda     #>~__COMMONMEM_SIZE__        ; Use -(__DATASIZE__+1)
	        sta     tmp1

		jsr	copyloop
		pla
		tax
		pla
		rts

		.segment "VECTORS"
		.addr	vector
		.addr	$C000		; our ROM mapping
		.addr	nmi_handler
