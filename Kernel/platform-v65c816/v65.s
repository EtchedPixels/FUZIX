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
	    .export vector

	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import unix_syscall_entry
	    .import kstack_top
	    .import istack_switched_sp
	    .import istack_top
	    .import _unix_syscall
	    .import _platform_interrupt
	    .import _kernel_flag
	    .import pushax

	    .import outcharhex
	    .import outxa
	    .import incaxy

	    .import _create_init_common

            .include "kernel.def"
            .include "../kernel816.def"
	    .include "zeropage.inc"

;
;	syscall is jsr [$00fe]
;
syscall	=  $FE

        .code

_trap_monitor:
	jmp	_trap_monitor

_trap_reboot:
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
	cli
	rts
irq_on:
	sei
	rts

init_early:
	; copy the stubs from bank 0 to all banks so we can keep the
	jsr _create_init_common
        rts

init_hardware:
        ; set system RAM size for test purposes
	rep #$10
	.i16
	ldx #8
	stx _ramsize
	ldx #512-64
	stx _procmem
	; TODO - correct vectors for the 816
	ldx #vector
	stx $FFFE
	ldx #<nmi_handler
	stx $FFFA
	ldx #syscall_entry
	stx #syscall

	rep #$10
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
;	Code that will live in each bank
;
	.segment "STUBS"
sigret:
	pla		; Unstack the syscall return pieces
	tax
	pla
	tay
	pla
	plp		; from original stack frame
	rts

; FIXME: add sig ret interrupt path


;
;	I/O logic
;


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
	ldy #00FE
	phy
	pld			; DP is now the I/O space
	
	ldy #512
	lda $34			; I/O data via DP
	sta 0,x			; stores into data (user) bank
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
	ldy #00FE
	phy
	pld			; DP is now the I/O space
	
	ldy #512
	lda 0,x			; load from data (user) bank
	sta $34			; I/O data via DP
	inx
	dey
	bne hd_read
	plb			; restore bank registers
	pld
	sep #$10
	.i8			; restore expected CPU state
	rts

	.bss

_hd_map:
	.res 1
