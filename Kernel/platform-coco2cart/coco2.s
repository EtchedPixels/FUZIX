	;
	; COCO2 platform
	;

	.module coco2

	; exported
	.globl _mpi_present
	.globl _mpi_set_slot
	.globl _cart_hash
	.globl map_kernel
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save
	.globl map_restore
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
	.globl _need_resched

	.globl _ramsize
	.globl _procmem
	.globl _bufpool
	.globl _discard_size

	; imported
	.globl unix_syscall_entry
;FIX	.globl fd_nmi_handler
	.globl null_handler
	.globl _vtoutput

	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	; exported debugging tools
	.globl _platform_monitor
	.globl _platform_reboot
	.globl outchar

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

	.area .text

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
	ldd #32
	std _procmem

	; Turn on PIA  CB1 (50Hz interrupt)
	lda 0xFF03
	ora #1
	sta 0xFF03
	jsr _vid256x192
	jsr _vtinit
	rts

        .area .common

_platform_reboot:
	orcc #0x10
	clr 0xFFBE
	lda #0x7e
	sta 0x0071		; in case IRQ left it looking valid
	jmp [0xFFFE]

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

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common
;
;	Our vectors live in a fixed block and are not banked out. Likewise
;	No memory management for now
;
_program_vectors:
	rts

map_kernel:
	pshs a
	lda #1
	sta romin
map_kr:
	clr $ffde	;	ROM in
	puls a,pc
	
map_process:
map_process_always:
map_process_a:
	clr romin
	clr $ffdf	;	ROM out
	rts

map_restore:
	pshs a
	lda romsave
	sta romin
	bne map_kr
	clr $ffdf
	puls a,pc
	
map_save:
	pshs a
	lda romin
	sta romsave
	puls a,pc
;
;	Helpers for the MPI and Cartridge Detect
;

;
;	oldslot = mpi_set_slot(uint8_t newslot)
;
_mpi_set_slot:
	tfr b,a
	ldb 0xff7f
	sta 0xff7f
	rts
;
;	int8_t mpi_present(void)
;
_mpi_present:
	lda 0xff7f	; Save bits
	ldb #0xff	; Will get back 33 from an MPI cartridge
	stb 0xff7f	; if the emulator is right on this
	ldb 0xff7f
	cmpb #0x33
	bne nompi
	clr 0xff7f	; Switch to slot 0
	ldb 0xff7f
	bne nompi
	incb
	sta 0xff7f	; Our becker port for debug will be on the default
			; slot so put it back for now
	rts		; B = 0
nompi:	ldb #0
	sta 0xff7f	; Restore bits just in case
	rts

	.area .text	; Must be clear of cartridge mapping areas
;
;	uint16_t cart_hash(void)
;
_cart_hash:
	pshs cc
	orcc #0x10
	ldx #0xC000
	ldd #0
	clr $FFBE	; Map cartridge
hashl:
	addd ,x++
	cmpx #0xC200
	bne hashl
	tfr d,x
	clr $FFBF	; Return to normality
	puls cc,pc


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
romin:
	.db 0
romsave:
	.db 0

	.area .bufpool
_bufpool:
	.ds 520*5		; initial buffers

	; Discard follows this so will be reclaimed

	.area .discard
_discard_size:
	.db	__sectionlen_.discard__/BUFSIZE
