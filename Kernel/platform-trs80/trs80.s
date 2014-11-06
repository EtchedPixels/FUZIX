;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
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
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl trap_illegal
            .globl outcharhex
	    .globl nmi_handler
	    .globl null_handler

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

trapmsg:    .ascii "Trapdoor: SP="
            .db 0
trapmsg2:   .ascii ", PC="
            .db 0
tm_user_sp: .dw 0

tm_stack:
            .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
tm_stack_top:

_trap_monitor:
	    ld a, #128
	    out (0x28), a
platform_interrupt_all:
	    ret

_trap_reboot:
	    ld a, #1
	    out (0x28), a

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

_ctc6845:				; registers in reverse order
	    .db 99, 80, 85, 10, 25, 4, 24, 24, 0, 9, 101, 9, 0, 0, 0, 0
init_early:
	    ld a, (_opreg)
	    out (0x84), a
	    ld a, (_modout)
	    out (0xEC), a

            ; load the 6845 parameters
	    ld hl, #_ctc6845
	    ld b, #1588
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x89), a		; data
	    inc hl
	    djnz ctcloop

   	    ; clear screen
	    ld hl, #0xF800
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0xF802
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
            ret

init_hardware:
            ; set system RAM size
            ld hl, #128
            ld (_ramsize), hl
            ld hl, #(128-64)		; 64K for kernel
            ld (_procmem), hl

	    ; 100Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

opsave:	    .db 0x36
_opreg:	    .db 0x36	; kernel map, 80 columns
_modout:    .db 0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
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

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

;
;	Fixed mapping set up for the TRS80 4/4P
;
;	Kernel runs mode 2, U64K/U32 mapped at L64K/U32
;
map_kernel_a:
	    ld a, i
	    push af
	    di
	    ld a, (_opreg)
	    and #0xAC
	    or #0x12
	    ld (_opreg), a
	    out (0x84), a
	    pop af
	    ret po
	    ei
            ret
map_kernel:
	    push af
	    call map_kernel_a
	    pop af
	    ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
	    call map_process_always_a
	    ret

map_process_always_a:
	    push af
	    ld a, (_opreg)
	    and #0xAC
	    or #0x43
	    ld (_opreg), a
	    out (0x84), a
	    pop af
            ret

map_process_always:
	    ld a, i
	    push af
	    di
	    call map_process_always_a
	    pop af
	    ret po
	    ei
	    ret

map_save:   push af
	    ld a, (_opreg)
	    and #0x53
	    ld (opsave), a
	    pop af
	    ret

map_restore:
	    push af
	    push bc
	    ld a, (opsave)
	    ld b, a
	    ld a, (_opreg)
	    and #0xAC
	    or b
	    ld (_opreg), a
	    out (0x84), a
	    ret
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            out (0x01), a
            ret

nap:
	; FIXME
	   ret
;
;	Idle the WD1772
;
_fd_idle:
	    in a, (0xF0)
	    rra
	    jr c, _fd_idle
	    ld l, a
	    ld h, #0
	    ret

;
;	See to a given track in C
;
_fd_seek:
	    ld a, c
	    out (0xF1), a		; track #
	    ld a, #0x1B			; Seek
	    out (0xF0), a
	    call nap
	    call _fd_idle
	    and #0x10
	    ret


;
;	Write a 256 byte block to disk. Need to add precomp etc to this
;	HL = pointer to data
;	A = bank info (0 kernel, !0 user). We don't handle swap to floppy.
;
;
_fd_write:
	    or a
	    jr z, _fd_kwrite
	    call map_process_always
_fd_kwrite:
	    ld bc, #0x00F3		; port F3, 256 bytes
	    ld a, i
	    push af
	    ld a, #0xAC			; Write
	    out (0xF0), a
	    call nap
_fd_writel:
	    in a, (0xF0)
	    bit 1, a
	    jr z, _fd_writec
	    di
_fd_writeb:
	    otir
	    ei
	    call _fd_idle
	    and #0x5C
fd_wout:
	    ld l, a
	    call map_kernel
	    pop af
	    ret po
	    ei
	    ret
_fd_writec: 
	    bit 2, a
	    jr nz, _fd_writel
	    ld a, #0xff
	    jr fd_wout



;
;	Read a 256 byte block from disk. Need to add precomp etc to this
;	HL = pointer to data
;	A = bank info (0 kernel, !0 user). We don't handle swap to floppy.
;
;
_fd_read:
	    or a
	    jr z, _fd_kwrite
	    call map_process_always
_fd_kread:
	    ld bc, #0x00F3		; port F3, 256 bytes
	    ld a, i
	    push af
	    ld a, #0x8C			; Read
	    out (0xF0), a
	    call nap
_fd_readl:
	    in a, (0xF0)
	    bit 1, a
	    jr z, _fd_readc
	    di
_fd_readb:
	    inir
	    ei
	    call _fd_idle
	    and #0x5C
fd_rout:
	    ld l, a
	    pop af
	    ret po
	    ei
	    ret
_fd_readc: 
	    bit 2, a
	    jr nz, _fd_readl
	    ld a, #0xff
	    jr fd_rout

_fdc_idle:
	; FIXME
	ret
;
;	Restore the current drive to track 0 (error recovery)
;
_fd_reset:
	    ld a, #0xB
	    out (0xF0), a
	    call nap
	    call _fdc_idle
	    and #0x10
	    ret
