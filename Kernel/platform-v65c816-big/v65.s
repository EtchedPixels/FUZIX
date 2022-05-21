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
	lda	#3
	sep	#$30
	.a8
	.i8

init_loop:
	sta	f:KERNEL_CODE_FAR+common_patch+1 ; destination bank
	phb				; save our bank (mvn will mess it)
	pha				; and count

	rep	#$30
	.a16
	.i16

	ldx	#$FF00
	txy
	lda	#$00FE
common_patch:
	mvn	#0,#KERNEL_CODE_BANK	; copy the block

	sep	#$30
	.a8
	.i8

	pla
	plb				; bank to kernel bank
	inc
	cmp	#128
	bne	init_loop
        rts

	.a8
	.i8

init_hardware:
        ; set system RAM size	(FIXME: dynamic probe)
	rep #$10
	.i16
	ldx #8192
	stx _ramsize
	ldx #8192-64
	stx _procmem

	sep #$10
	.i8

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
	sta f:$0000FE20
	rts


;
;	I/O logic
;

	.export _hd_kmap
	.export _hd_read_data
	.export _hd_write_data
	.export _peek,_poke

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
	sta f:KERNEL_CODE_FAR+hd_rpatch+1	; destination is bank we want
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
	sta f:KERNEL_CODE_FAR+hd_wpatch+2	; source is bank we want
	phb			; bank will be corrupted

	lda  #$34		; DMA port
	sta f:$00FE11		; point it at the disk port

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

	.data

_hd_kmap:
	.res 1

	.code

;
;	For non bank 0 we can't just poke addresses from C but need helpers.
;	We pass a single argument of dataoffset,data so we get a single
;	value in XA for speed (X is offset A data)
;
_poke:
	stx ptr1
	ldx #$FE
	stx ptr1+1
	ldx #0
	phb
	phx
	plb
	sta (ptr1)
	plb
	rts
_peek:
	sta ptr1
	lda #$FE
	sta ptr1+1
	phb
	lda #0
	pha
	plb
	lda (ptr1)
	plb
	ldx #0
	rts


	.segment "STUBS"
	.export jmpvec
	.export callax

;
;	Hack to deal with CC65 not supporting split I/D properly. It tries
;	to generate stores to jmpvec+1/+2 them jsr jmpvec assuming they are
;	in fact all in the same bank.
;
;	FIXME: we need to save 2:jmpvec+1/+2 across interrupts
;
callax:		; FIXME: optimise
	.a8
	.i8

	sta     jmpvec+1
        stx     jmpvec+2
jmpvec:
	.a8
	.i8
	rep #$10
	.i16
	ldx jmpvec+1		; in bank 2 not bank 1
	dex			; as rts will inc
	phx
	sep #$10
	.i8
	rts
