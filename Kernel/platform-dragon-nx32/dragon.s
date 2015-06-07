	;
	; common Dragon platform
	;

		.module dragon

		; exported
		.globl _mpi_present
		.globl _mpi_set_slot
		.globl _cart_hash

		; imported
		.globl unix_syscall_entry
		.globl fd_nmi_handler
		.globl size_ram
		.globl null_handler
		.globl _vid256x192
		.globl _vtoutput

		; exported debugging tools
		.globl _trap_monitor
		.globl _trap_reboot
		.globl outchar
		.globl _di
		.globl _ei
		.globl _irqrestore

            include "kernel.def"
            include "../kernel09.def"


		.area .vectors
	;
	;	At 0x100 as required by the Dragon ROM
	;
		    jmp badswi_handler			; 0x100
		    jmp badswi_handler			; 0x103
		    jmp unix_syscall_entry 		; 0x106
		    jmp fd_nmi_handler			; 0x109
		    jmp interrupt_handler		; 0x10C
		    jmp firq_handler			; 0x10F

	.area .text

init_early:
	ldx #null_handler
	stx 1
	lda #0x7E
	sta 0
	sta 0x0071		; BASIC cold boot flag
	rts

init_hardware:
	jsr size_ram
	; Turn on PIA  CB1 (50Hz interrupt)
	lda 0xFF03
	ora #1
	sta 0xFF03
	jsr _vid256x192
	jsr _vtinit
	rts


; old p6809.s stuff below

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl _need_resched


            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl unix_syscall_entry
	    .globl fd_nmi_handler

            .area .common

_trap_reboot:
	    orcc #0x10
	    clr 0xFFBE
	    jmp [0xFFFE]

_trap_monitor:
	    orcc #0x10
	    bra _trap_monitor

_di:
	    tfr cc,b		; return the old irq state
	    orcc #0x10
	    rts
_ei:
	    andcc #0xef
	    rts

_irqrestore:			; B holds the data
	    tfr b,cc
	    rts

            .area .text


;
;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW


            .area .common

;
;	In the Dragon nx32 case our vectors live in a fixed block
; 	and is not banked out.
;
_program_vectors:
	    rts

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

_need_resched: .db 0
