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
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl _fd_bankcmd
	    .globl platform_interrupt_all
	    .globl map_save
	    .globl map_restore
	    .globl _need_resched

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl _inint
            .globl _tty_inproc
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex
	    .globl nmi_handler
	    .globl null_handler
	    .globl interrupt_handler
	    

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_platform_monitor:
	    ld a, #128
	    out (29), a
platform_interrupt_all:
	    ret

_platform_reboot:
	    ld a, #1
	    out (29), a

;
;	We need the right bank present when we cause the transfer
;
_fd_bankcmd:pop de		; return
	    pop bc		; command
	    pop hl		; bank
	    push hl
	    push bc
	    push de		; fix stack
	    ld a, (_int_disabled)
	    di
	    push af		; save DI state
	    call map_process	; (HL) holds our bank
	    ld a, c		; issue the command
	    out (13), a		;
	    call map_kernel	; return to kernel mapping
	    pop af
	    or a
	    ret nz
	    ei
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, #192			; 192 * 256 bytes (48K)
	    out (22), a			; set up memory banking
	    ld a, #2	
	    out (20), a			; 2 segments
            ret

init_hardware:
            ; set system RAM size
            ld hl, #128
            ld (_ramsize), hl
            ld hl, #(128-64)		; 64K for kernel
            ld (_procmem), hl

	    ld a, #1
	    out (27), a			; 100Hz timer on

            ; set up interrupt vectors for the kernel
	    ; (also sets up common memory in page 0x000F which is unused)
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
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page pointer or NULL for kernel
            push hl ; put stack back as it was
            push de

	    call map_process

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

            ; put the MMU back as it was -- we're in kernel mode so this is predictable

	    ;
	    ; Map in the kernel
	    ;
map_kernel:
	    xor a
	    out (21), a
            ret
	    ;
	    ; Map in the current user process
	    ;
map_process_always:
	    ld a, #1
	    out (21), a
	    ret
	    ;
	    ; Map in a process. HL pointers to the base of the 4 bytes of
	    ; page data or is NULL for kernel mapping
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
	    ;
	    ; Map the process page in A. Handy local helper rather than generally
	    ; useful in all cases
	    ;
map_process_a:
	    out (21), a
            ret

map_save:
	    push af
	    in a, (21)
	    ld (saved_map), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (saved_map)
	    out (21), a
	    pop af
	    ret

saved_map:  .db 0

_need_resched:
	    .db 0


; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x01), a
            ret
