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
	    .globl _kernel_flag

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar
	    .globl _di
	    .globl _ei
	    .globl _irqrestore

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl unix_syscall_entry
	    .globl nmi_handler

            include "kernel.def"
            include "../kernel09.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

trapmsg:    .ascii "Trapdoor: SP="
            .db 0
trapmsg2:   .ascii ", PC="
            .db 0
tm_user_sp: .dw 0

_trap_monitor:
	    orcc #0x10
	    bra _trap_monitor

_trap_reboot:
	    lda 0xff90
	    anda #0xfc		; map in the ROM
	    jmp 0

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

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area .text

init_early:
            rts

init_hardware:
            ; set system RAM size
	    ldd #96
	    std _ramsize
	    ldd #32
	    std _procmem
            rts


	    .area .vectors
;
;	At 0x100 as required by the Dragon ROM
;
	    jmp badswi_handler			; 0x100
	    jmp badswi_handler			; 0x103
	    jmp unix_syscall_entry 		; 0x106
	    jmp nmi_handler			; 0x109
	    jmp interrupt_handler		; 0x10C
	    jmp firq_handler			; 0x10F

;
;	Included if we are replacing the basic ROM and magically mapped from
;	the ROM top (0xBFF0->FFF0)
;
	    .area .romvectors

	    .dw 0x3634		; Reserved
	    .dw 0x0100
	    .dw 0x0103
	    .dw 0x010f
	    .dw 0x010c
	    .dw 0x0106
	    .dw 0x0109
	    .dw 0xC000		; Unused (reset)

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW


            .area _COMMONMEM

;
;	In the Dragon64 case our vectors live in a fixed block
;
_program_vectors:
	    rts

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
;
;
;	All registers preserved
;
map_process_always:
	    pshs a
map_process_2:
	    lda 0xffdf			; RAM please
	    lda #0
	    sta cart_map
	    puls a, pc    

map_process:
	    cmpx #0
	    bne map_process_always
;
;	Map in the kernel below the current common, all registers preserved
;
map_kernel:
	    pshs a
map_kernel_2:
	    lda 0xffde			; RAM out (cart in hopefully)
	    lda #1
	    sta cart_map
	    puls a, pc

map_restore:
	    pshs a
	    lda saved_map
	    cmpa #0
	    beq map_process_2
	    bra map_kernel_2
;
;	Save the current mapping.
;
map_save:
	    pshs a
	    lda cart_map
	    sta saved_map
	    puls a,pc

saved_map:  .db 0
cart_map:   .db 0	    

; outchar: Wait for UART TX idle, then print the char in a

outchar:
	    pshs b
outcharw:
	    ldb 0xff05
	    bitb #0x04
	    beq outcharw
	    sta 0xff04
	    puls b,pc

_kernel_flag: .db 1
