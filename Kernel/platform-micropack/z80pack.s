;
;	Z80Pack hardware support
;
;
;	This goes straight after udata for common. Because of that the first
;	256 bytes get swapped to and from disk with the uarea (512 byte disk
;	blocks). This isn't a problem but don't put any variables in here.
;
;	If you make this module any shorter, check what follows next
;


            .module z80pack

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
            .globl _system_tick_counter

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl platform_interrupt_all

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (not meaningful on swap only)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	    ld a, #128
	    out (29), a
platform_interrupt_all:
	    ret

_trap_reboot:
	    ld a, #1
	    out (29), a

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ret

init_hardware:
            ; set system RAM size
            ld hl, #64
            ld (_ramsize), hl
            ld hl, #32			; 64K for kernel
            ld (_procmem), hl

	    ld a, #1
	    out (27), a			; 100Hz timer on

            ; set up interrupt vectors for the kernel
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM


_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off

            ; write zeroes across all vectors
            ld hl, #0
            ld de, #1
            ld bc, #0x007f ; program first 0x80 bytes only
            ld (hl), #0x00
            ldir

            ; now install the interrupt vector at 0x0038
            ld a, #0xC3 ; JP instruction
            ld (0x0038), a
            ld hl, #interrupt_handler
            ld (0x0039), hl

            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ; Set vector for jump to NULL
            ld (0x0000), a   
            ld hl, #null_handler  ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; our platform has a "true" common area, if it did not we would
	    ; need to copy the "common" code into the common area of the new
	    ; process.

	    ; falls through

	    ; Map functions are trivial (but will need bank switchers
	    ; elsewhere)
map_kernel:
map_process:
map_process_always:
map_save:
map_restore:
	    ret	    

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x01), a
            ret
