	;
	; common RC2014 6809 code
	;

	.module rc2014_6809

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
	.globl _discard_size

	; imported
	.globl unix_syscall_entry
	.globl size_ram
	.globl null_handler

	; exported debugging tools
	.globl _platform_monitor
	.globl _platform_reboot
	.globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

        include "kernel.def"
        include "../kernel09.def"


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
_discard_size:
	.db	__sectionlen_.discard__/BUFSIZE

init_early:
	rts

init_hardware:
	jsr size_ram
	; Turn on timer TODO
	rts

        .area .common

_platform_reboot:
	; TODO
_platform_monitor:
	orcc #0x10
	bra _platform_monitor

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
