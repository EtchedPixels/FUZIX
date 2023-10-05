;
;	    v65 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors

            ; exported debugging tools
            .export _plt_monitor
	    .export _plt_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import syscall_vector
	    .import _vtinit
	    .import kstack_top
	    .import istack_switched_sp
	    .import istack_top
	    .import _kernel_flag
	    .import pushax

	    .import outcharhex
	    .import outxa
	    .import incaxy

            .include "kernel.def"
            .include "../kernel816.def"
	    .include "zeropage.inc"

	.p816
	.a8
	.i8
;
;	syscall is jsr [$00fe]
;
syscall	=  $FE

        .code

_plt_monitor:
	jmp	_plt_monitor

_plt_reboot:
	sep	#$30
	.a8
	.i8
	lda	#$A5
	sta	$FE40		; Off
	jmp	_plt_reboot	; FIXME: original ROM map and jmp

___hard_di:
	php
	sei			; Save old state in return to C
	pla			; Old status
	rts

___hard_ei:
	cli			; on 6502 cli enables IRQs!!!
	rts

___hard_irqrestore:
	and	#4		; IRQ flag
	beq	irq_on
	sei
	rts
irq_on:
	cli
	rts

;
;	This could go in discard once we make that useful FIXME
;
init_early:
	lda	#1
	sep	#$30
	.a8
	.i8

init_loop:
	sta	common_patch+1		; destination bank
	phb				; save our bank (mvn will mess it)
	pha				; and count

	rep	#$30
	.a16
	.i16

	ldx	#$FF00
	txy
	lda	#$00FE
common_patch:
	mvn	#KERNEL_FAR,0		; copy the block

	sep	#$30
	.a8
	.i8

	pla
	plb				; bank to kernel bank
	inc
	cmp	#8
	bne	init_loop
        rts

	.a8
	.i8

init_hardware:
        ; set system RAM size	(FIXME: dynamic probe)
	rep #$10
	.i16
	ldx #512
	stx _ramsize
	ldx #512-64
	stx _procmem

	sep #$10
	.i8

	jsr _vtinit

	rts
;
;	We did this at early boot when we set up the vectors and copied
;	stubs everywhere. Only thing we needed in each bank vector wise
;	was syscall (if we keep to 6502 style syscall)
;
_program_vectors:
	rts

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers
outchar:
	sta $0000FE20
	rts


;
;	I/O logic
;

	.export _hd_kmap
	.export _hd_read_data
	.export _hd_write_data

;
;	Disk copier
;
;	XA = ptr, length always 512, page in globals
;

_hd_read_data:
	xba
	txa			; A now holds the 16 bit buffer address
	xba
	rep #$10
	.i16
	tay			; Y is target
	ldx #$0			; FF0000 is source for block transfer
				; (range all maps to disk I/O port)
	lda _hd_kmap
	sta hd_rpatch+1		; destination is bank we want
	phb			; bank will be corrupted

	lda  #$34		; DMA port
	sta f:$00FE11		; point it at the disk port

	rep #$30
	.a16

	lda #$200-1		; 1 sector
hd_rpatch:
	mvn #$FF,#$FF
	plb
	sep #$30		; go anywhere
	.a8
	.i8

	lda #0
	sta f:$00FE11		; ensure any DMA window I/O doesn't

	rts

_hd_write_data:
	xba
	txa			; A now holds the 16 bit buffer address
	xba
	rep #$10
	.i16
	tax			; X is source
	ldy #$0			; FF0000 is target for block transfer
				; (range all maps to disk I/O port)
	lda _hd_kmap
	sta hd_wpatch+2		; source is bank we want

	lda  #$34		; DMA port
	sta f:$00FE11		; point it at the disk port

	phb			; bank will be corrupted

	rep #$30
	.a16

	lda #$200-1		; 1 sector

hd_wpatch:
	mvn #$FF,#$FF
	plb
	sep #$30		; go anywhere
	.a8
	.i8

	lda #0
	sta f:$00FE11		; ensure any DMA window I/O doesn't

	rts

;
;	Frame buffer driver. On entry A is the char
;
	.import _fb_off
	.import _fontdata_8x8
	.import _fb_count

	.export _charprint
	.export _scroll_up
	.export _scroll_down
	.export _do_clear_bytes
	.export _cursor_off
	.export _cursor_disable
	.export _do_cursor_on

_charprint:
	rep #$30
	.a16
	.i16
	and #$ff
	asl
	asl
	asl
	tax

	ldy _fb_off

	sep #$20
	.a8

	phb
	lda #$fe
	pha
	plb

;
;	For a bank 0 kernel or at least font we could maybe instead
;	of the tax do phd pha pld and use 0-7 of ZP as the font
;	source (saves 16 clocks minus the extra setup costs). Otherwise it
;	saves one clock per 1 byte if you can put the font in the same
;	bank as the video ram by using AI/X not ALI/X
;

	; 5 clocks
	lda f:KERNEL_FAR+_fontdata_8x8-32*8,x
	; 5 clocks
	sta a:0,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+1,x
	sta a:80,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+2,x
	sta a:160,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+3,x
	sta a:240,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+4,x
	sta a:320,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+5,x
	sta a:400,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+6,x
	sta a:480,y
	lda f:KERNEL_FAR+_fontdata_8x8-32*8+7,x
	sta a:560,y

	plb

	sep #$10
	.i8
	rts

_scroll_up:
	phb
	rep #$30
	.a16
	.i16
	;
	;	Do the scroll. For once something we are good at
	;
	ldx #640
	ldy #0
	lda #15360-1		; 192 pixel rows
	mvn #$FE,#$FE
	sep #$30
	.a8
	.i8
	plb
	rts

_scroll_down:
	phb
	rep #$30
	.a16
	.i16
	ldx #16000-1
	ldy #15360-1
	lda #15360-1
	mvp #$FE,#$FE
	sep #$30
	.a8
	.i8
	plb
	rts

;
;	We use this both to clear short bursts 8 times (clear across)
;	and to clear lines (80 bytes * 8 per row * lines)
;
_do_clear_bytes:
	rep #$10
	.i16
	ldx _fb_off
	ldy _fb_off
	iny
	lda #0
	sta f:$FE0000,x
	rep #$20
	.a16
	lda _fb_count
	phb
	mvn #$FE,#$FE
	sep #$30
	.a8
	.i8
	plb
	rts

	.a8
	.i8

_cursor_off:
	rep #$10
	.i16
	ldx cursorpos
	bra cursormod

	.a8
	.i8

_do_cursor_on:
	rep #$10
	.i16
	ldx _fb_off
	stx cursorpos
cursormod:
	phb
	lda #$fe
	pha
	plb
	lda #$ff
	eor a:0,x
	sta a:0,x
	lda #$ff
	eor a:80,x
	sta a:80,x
	lda #$ff
	eor a:160,x
	sta a:160,x
	lda #$ff
	eor a:240,x
	sta a:240,x

	lda #$ff
	eor a:320,x
	sta a:320,x
	lda #$ff
	eor a:400,x
	sta a:400,x
	lda #$ff
	eor a:480,x
	sta a:480,x
	lda #$ff
	eor a:560,x
	sta a:560,x

	plb
	sep #$10
	.i8
_cursor_disable:
	rts

	.data

cursorpos:
	.word $7FF0		; off screen bytes to poop on
_hd_kmap:
	.res 1
