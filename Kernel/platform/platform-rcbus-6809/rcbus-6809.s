	;
	; common RC2014 6809 code
	;

	.module rcbus_6809

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
	.globl _need_resched


            ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl unix_syscall_entry

	; exported
	.globl _bufpool

	; imported
	.globl unix_syscall_entry
	.globl size_ram
	.globl null_handler

	; exported debugging tools
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

        include "kernel.def"
        include "../../cpu-6809/kernel09.def"


	.area .vectors

	.dw 0				; reserved
	.dw badswi_handler		; SWI3
	.dw badswi_handler		; SWI2
	.dw firq_handler		; FIR
	.dw interrupt_handler		; IRQ
	.dw unix_syscall_entry 		; SWI
	.dw nmi_handler			; NMI
	.dw 0				; RESET (never used)

	.area	.buffers
	;
	;	We use the linker to place these just below
	;	the discard area
	;
_bufpool:
	.ds	BUFSIZE*NBUFS

	;	And expose the discard buffer count to C - but in discard
	;	so we can blow it away and precomputed at link time
	;
	;	TODO: this wants to change to blow away anything below
	;	common.
	;
	.area	.discard

init_early:
	rts

init_hardware:
	jsr size_ram
	ldx #$FE60		; 6840 PTM at I/O 0x60
	lda #1
	sta 1,x			; Enable CR1, set CR2 to be
				; square wave no IRQ, refclk, 16bit
	lda #0x01
	sta ,x			; Reset the chip, CR1 config doesn't matter
				; providing output is disabled
	ldd #CLKVAL
	std 6,x			; Timer 3
	clr 1,x			; CR2 square, no IRQ, no out, no input,
				; CR3 accessible
	lda #0x43		; counter mode, count E clocks, prescale
				; IRQ on
	sta ,x
	lda #0x01
	sta 1,x			; Back to CR1
	clr ,x			; out of reset
	rts

        .area .common

_plt_reboot:
	; TODO
_plt_monitor:
	orcc #0x10
	bra _plt_monitor

___hard_di:
	tfr cc,b		; return the old irq state
	orcc #0x10
	rts
___hard_ei:
	andcc #0xef
	rts

___hard_irqrestore:		; B holds the data
	tfr b,cc
	rts


        .area .common

;
;	Our vectors are in a single fixed bank so no work is needed. Will
;	change if we move to properly using the paging.
;
_program_vectors:
	rts

;
;	Nothing to do here - we don't use SWI or FIRQ
;
firq_handler:
badswi_handler:
	rti

;
;	debug via serial console port 1
;
outchar:
	ldb $FEC5
	andb #0x20
	beq outchar
	sta $FEC0
	rts

	    .area .commondata

_need_resched:
	.db 0
