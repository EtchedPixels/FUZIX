;
;	    v65 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors

            ; exported debugging tools
            .export _trap_monitor
	    .export _trap_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import syscall_vector
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

_trap_monitor:
	jmp	_trap_monitor

_trap_reboot:
	sep	#$30
	.a8
	.i8
	lda	#$A5
	sta	$FE40		; Off
	jmp	_trap_reboot	; FIXME: original ROM map and jmp

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
	mvn	KERNEL_FAR,0		; copy the block

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
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	AX = ptr, length always 512, page in globals
;

_hd_read_data:
	sta ptr3
	stx ptr3+1
	phd
	phb
	rep #$10
	.i16
	ldx ptr3		; buffer address
	lda _hd_kmap		; page number
	pha
	plb			; data now points into user app
	ldy #$FE00
	phy
	pld			; DP is now the I/O space
	
	ldy #512
hd_read:
	lda $34			; I/O data via DP
	sta a:$0000,x		; stores into data (user) bank
	inx
	dey
	bne hd_read
	plb			; restore bank registers
	pld
	sep #$10
	.i8			; restore expected CPU state
	rts

_hd_write_data:
	sta ptr3
	stx ptr3+1
	phd
	phb
	rep #$10
	.i16
	ldx ptr3		; buffer address
	lda _hd_kmap		; page number
	pha
	plb			; data now points into user app
	ldy #$FE00
	phy
	pld			; DP is now the I/O space
	
	ldy #512
hd_write:
	lda a:$0000,x		; load from data (user) bank
	sta $34			; I/O data via DP
	inx
	dey
	bne hd_write
	plb			; restore bank registers
	pld
	sep #$10
	.i8			; restore expected CPU state
	rts

	.bss
_hd_kmap:
	.res 1
