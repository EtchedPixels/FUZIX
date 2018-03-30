;
;	    6809 Simulation Platform 
;

            .module p6809

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl _need_resched

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar
	    .globl ___hard_di
	    .globl ___hard_ei
	    .globl ___hard_irqrestore

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl unix_syscall_entry
	    .globl fd_nmi_handler

            include "kernel.def"
            include "../kernel09.def"

            .area .common

_platform_reboot:
_platform_monitor:
	    cwai #0
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

            .area .text

init_early:
            rts

init_hardware:
            ; set system RAM size
	    ldd #64
	    std _ramsize
	    ldd #56
	    std _procmem
	    ; set up SAM vector for kernel
	    ldx #0
	    jsr _program_vectors
	    ; Turn on PIA  CB1 (50Hz interrupt)
	    lda 0xFF03
	    ora #1
	    sta 0xFF03
            rts


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

;
;	Included if we are replacing the basic ROM and magically mapped from
;	the ROM top (0xBFF0->FFF0)
;
	    .area .romvectors

	    .dw 0x3634		; Reserved
            .dw 0x0100		; SWI3
	    .dw 0x0103		; SWI2
	    .dw 0x010f		; FIRQ
	    .dw 0x010c		; IRQ
	    .dw 0x0106		; SWI
	    .dw 0x0109		; NMI
	    .dw 0xC000		; Unused (reset)

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW


            .area .common

;
;	In the Dragon64 case our vectors live in a fixed block
;
_program_vectors:
	    pshs cc
	    orcc #0x10
	    lda #0x7E
	    sta 0		; NULL pointer trap
; FIXME: add a target address for NULL execution
	    ldd #0xFFD5
	    cmpx #0		; Kernel or user ?
	    beq setsamvec
	    ldx #0x100
vecl:	    SAM_KERNEL		; Copy vectors
	    ldd ,x
	    SAM_USER
	    std ,x+
	    cmpx #0x112
	    bne vecl
	    ; in SAM_USER so we hit the right page
	    ldd #0xFFD4
setsamvec:  std 4		; [4] can now be used for SAM_RESTORE
	    SAM_KERNEL
	    puls cc,pc

;
;	FIXME:
;
firq_handler:
badswi_handler:
	    rti

;
;	The Dragon mapping is a bit tangled
;
;	SAM ffd4/d5 reading turns off/on the upper 32K of RAM at 0x0-0x7FFF
;	SAM ffde/ffdf makes it appear at 0x8000-0xffef (ff00... is  hardwired)
;	PIA 1 B side data reg (ff22) bit 2 switches between the two ROMs
;
;	This basically means that all the mapping has to be inlined. If we
;	ever support the DragonPlus we'll need to rather carefully address
; 	the case of flipping user bank between 0/A/B
;
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	    rts

; outchar: Wait for UART TX idle, then print the char in a

outchar:
;	    pshs b
;outcharw:
	    pshs x
	    ldx traceptr
	    cmpa #0x60		; mangle for 6847 VDU charset
	    blo lc
	    suba #0x20
lc	    anda #0x3F
	    sta ,x+
	    stx traceptr
	    puls x,pc
;	    ldb 0xff05
;	    bitb #0x04
;	    beq outcharw
;	    sta 0xff04
;	    puls b,pc

	    .area .data

_need_resched: .db 0
traceptr:
	   .dw 0x6000
