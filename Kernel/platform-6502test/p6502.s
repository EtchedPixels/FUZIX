;
;	    6502 Build testing
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_process
	    .export map_process_always
	    .export map_save
	    .export map_restore

            ; exported debugging tools
            .export _trap_monitor
            .export outchar
	    .export _di
	    .export _ei
	    .export _irqrestore

            .include "kernel.def"
            .include "../kernel02.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

trapmsg:    .asciiz "Trapdoor: SP="
trapmsg2:   .asciiz ", PC="
tm_user_sp: .word 0

_trap_monitor:
;	    orcc #0x10
;	    bra _trap_monitor

_trap_reboot:
;	    lda 0xff90
;	    anda #0xfc		; map in the ROM
;	    jmp 0

_di:
;	    tfr cc,b		; return the old irq state
;	    orcc #0x10
	    rts
_ei:
;	    andcc #0xef
	    rts

_irqrestore:			; B holds the data
;	    tfr b,cc
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
            rts

init_hardware:
;            ; set system RAM size
;	    ldd #256
;	    std _ramsize
;	    ldd #192
;	    std _procmem
;
;	    ; Our vectors are in high memory unlike Z80 but we still
;	    ; need vectors
;	    ldx #0
;            jsr _program_vectors

            rts


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .segment "COMMONMEM"

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
;            orcc #0x10	 	; di just to be sure

;	    jsr map_process

;	    ldx #0xFFF2
;	    ldd #badswi_handler
;	    std ,y++
;	    std ,y++			; SWI2 and 3 both bad SWI
;	    ldd #firq_handler
;	    std ,y++
;            ldd #interrupt_handler
;	    std ,y++
;            ldd #unix_syscall_entry
;	    stx ,y++
;	    ldd #nmi_handler
;	    stx ,y
;	    jsr map_kernel
	    rts

;
;	Userspace mapping pages 7+  kernel mapping pages 3-5, first common 6
;
;
;	All registers preserved
;
map_process_always:
;	    pshs y,u
;	    ldx #U_DATA__U_PAGE
;	    jsr map_process_2
;	    puls y,u,pc
;
;	HL is the page table to use, A is eaten, HL is eaten
;
map_process:
;	    cmpx #0
;	    bne map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;
map_kernel:
;
;	Two MMU mappings is pure luxury
;
;	Kernel map was set up by boot loader, just flip to it
;
;	    lda 0xff91			; INIT1, use 0xFFA8 maps
;	    ora #0x01
;	    sta 0xff91
	    rts
;
;	User is in the FFA0 map with the top 8K as common
;
;	As the core code currently does 16K happily but not 8 we just pair
;	up pages
;
map_process_2:
;	    pshs x,y,a
;	    ldy #0xffa0			; MMU user map. We can fiddle with
;	    lda ,x+			; this to our hearts content
;	    sta ,y+			; as it's not live yet
;	    inca
;	    sta ,y+
;	    lda ,x+
;	    sta ,y+
;	    inca
;	    sta ,y+
;	    lda ,x+
;	    sta ,y+
;	    inca
;	    sta ,y+
;	    lda ,x+
;	    sta ,y+
;	    inca	
;	    sta ,y
;	    lda 0xff91
;	    anda #0xfe
;	    sta 0xff91			; new mapping goes live here
;	    puls x,y,a,pc		; so had better include common!
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
;	We cheat somewhat. We have two mapping sets, so just remember
;	which space we were in
;
map_restore:
;	    pshs a
;	    lda 0xff91
;	    ora saved_map
;	    sta 0xff91
;	    puls a,pc
;	    

;	Save the current mapping.
;
map_save:
;	    pshs a
;	    lda 0xff91
;	    anda #1
;	    sta saved_map
;	    puls a,pc

saved_map:  .dbyt 0
	    

; outchar: Wait for UART TX idle, then print the char in a

outchar:
;	    pshs b
outcharw:
;	    ldb 0xffa0
;	    bitb #0x02
;	    beq outcharw
;	    sta 0xffa1
;	    puls b,pc
