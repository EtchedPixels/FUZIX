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


            .module px4plus

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
	    .globl _vtinit

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    ; RAM drive
	    .globl _hd_xferin
	    .globl _hd_xferout
	    .globl _ramd_off
	    .globl _ramd_size
	    .globl _ramd_uaddr


            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (not unmapped when we flip to ROM)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

; FIXME: figure out how to reboot into CP/M
_trap_monitor:
_trap_reboot:
	    di
	    halt
platform_interrupt_all:
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ret

init_hardware:
            ; set system RAM size
	    ; Not strictly correct FIXME: set right when ROM the image
            ld hl, #64
            ld (_ramsize), hl
            ld hl, #32			; 32K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; IRQ enables
	    ld a, #0xB		; OVF (timer), RXRDY (gapnio), 7508
	    out (0x04), a

	    call _vtinit
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

	    ; Map functions are trivial for now
	    ; FIXME: will need to play with BANKR once we ROM the image
map_kernel: 
map_process:
map_process_always:
map_save:
map_restore:
	    ret	    

; outchar: Wait for UART TX idle, then print the char in A
outchar:
	    push af
outcharw:
	    in a, (0x15)
	    bit 0, a
	    jr z, outcharw
	    pop af
	    out (0x14), a	 
            ret

;
;	RAM disc
;
_hd_xferin:
;
;	Load the full 24 bit address to start
;
	xor a		;	 always aligned
	out (0x90), a
	ld hl, #_ramd_off + 1
	ld de, (_ramd_size)
hd_xiloop:
	ld a, (hl)
	out (0x91), a
	inc hl
	ld a, (hl)
	out (0x92), a
	ld bc, #0x93
;
;	With a 256 byte block it autoincrements
;
	ld hl, (_ramd_uaddr)
	inir
	dec d
	ret z
;
;	Move on a block
;
	ld (_ramd_uaddr), hl
	ld hl, #_ramd_off + 1
	inc (hl)
	jr nz, hd_xiloop
	inc hl
	inc (hl)
	dec hl
	jr hd_xiloop

_hd_xferout:
;
;	Load the full 24 bit address to start
;
	xor a		;	 always aligned
	out (0x90), a
	ld hl, #_ramd_off + 1
	ld de, (_ramd_size)
hd_xoloop:
	ld a, (hl)
	out (0x91), a
	inc hl
	ld a, (hl)
	out (0x92), a
	ld bc, #0x93
;
;	With a 256 byte block it autoincrements
;
	ld hl, (_ramd_uaddr)
	otir
	dec d
	ret z
;
;	Move on a block
;
	ld (_ramd_uaddr), hl
	ld hl, #_ramd_off + 1
	inc (hl)
	jr nz, hd_xoloop
	inc hl
	inc (hl)
	dec hl
	jr hd_xiloop
