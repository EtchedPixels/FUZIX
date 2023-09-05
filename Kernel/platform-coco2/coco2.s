	;
	; COCO2 platform
	;

	.module coco2

	; exported
	.globl _mpi_present
	.globl _mpi_set_slot
	.globl _cart_hash
	.globl map_kernel
	.globl map_proc
	.globl map_proc_a
	.globl map_proc_always
	.globl map_save
	.globl map_restore
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
	.globl _need_resched

	.globl _ramsize
	.globl _procmem

	; imported
	.globl unix_syscall_entry
	.globl null_handler
	.globl _vtoutput

	; exported debugging tools
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	include "kernel.def"
	include "../kernel09.def"


		.area .vectors
	;
	;	At 0x100 as required by the COCO ROM
	;
	jmp badswi_handler			; 0x100
	jmp badswi_handler			; 0x103
	jmp unix_syscall_entry 			; 0x106
	; FIXME: 109 needs to handle bad NMI!
	jmp badswi_handler			; 0x109
	jmp interrupt_handler			; 0x10C
	jmp firq_handler			; 0x10F

	.area .discard

init_early:
	ldx #null_handler
	stx 1
	lda #0x7E
	sta 0
	sta 0x0071		; BASIC cold boot flag (will get trashed by
				; irq stack but hopefully still look bad)
	;
	;	Move the display
	;
	ldx #0xFFC7
	clr ,x+			; Set F0
	clr ,x++		; Clear F1-F6
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	rts

init_hardware:
	ldd #64
	std _ramsize
	ldd #28
	std _procmem

	; Turn on PIA  CB1 (50Hz interrupt)
	lda 0xFF03
	ora #1
	sta 0xFF03
	jsr _vtinit

	; NTSC or PAL/SECAM ?
	ldx	#0
	lda	$ff02
waitvb0
	lda	$ff03
	bpl	waitvb0		; wait for vsync
	lda	$ff02
waitvb2:
	leax	1,x		; time until vsync starts
	lda	$ff03
	bpl	waitvb2
	stx	_framedet
	rts

	.globl _framedet

_framedet:
	.word	0

        .area .page1
; Borrow a tiny bit of page1 to get this low so it can turn the ROM back on

_plt_reboot:
	orcc #0x10
	clr 0xFFBE
	lda #0x7e
	sta 0x0071		; in case IRQ left it looking valid
	jmp [0xFFFE]

	.area .common
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

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common
;
;	Our vectors live in a fixed block and are not banked out. Likewise
;	No memory management for now
;
_program_vectors:
map_kernel:
map_restore:
map_proc:
map_proc_always:
map_proc_a:
map_save:
	rts

	.area .common
;
;	FIXME:
;
firq_handler:
badswi_handler:
	rti

;
;	debug via printer port
;
outchar:
	sta 0xFF02
	lda 0xFF20
	ora #0x02
	sta 0xFF20
	anda #0xFD
	sta 0xFF20
	rts

	.area .common

_need_resched:
	.db 0
