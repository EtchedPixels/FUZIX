;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl plt_interrupt_all

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

            ; exported debugging tools
            .globl _plt_monitor
            .globl _plt_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl outcharhex
	    .globl fd_nmi_handler
	    .globl null_handler
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl _opreg
	    .globl _modout
	    .globl _int_disabled

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl _bufpool

            .include "kernel.def"
            .include "../kernel-z80.def"

	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_int_disabled:
	    .db 1

_plt_monitor:
	    di
	    halt

plt_interrupt_all:
	    ret

;
;	We write the following into low memory
;	out (0x84),a
;	inc a
;	out (0x9c),a
;	.. then resets into ROM (see the technical reference manual)
;
_plt_reboot:
	   di
	   ld hl,#0x84D3		; out (0x84),a
	   ld (0),hl
	   ld hl,#0xD33C		; inc a, out (
	   ld (2),hl
	   ld a,#0x9C			; 9c),a
	   ld (4),a
	   xor a
	   rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

_ctc6845:				; registers in order
	    .db 99, 80, 85, 10, 25, 4, 24, 24, 0, 9, 101, 9, 0, 0, 0, 0
init_early:
	    ld a, #0x24			; uart rx, timer
	    out (0xE0), a
	    ld a, (_opreg)
	    out (0x84), a
	    ld a, (_modout)
	    out (0xEC), a

            ; load the 6845 parameters
	    ld hl, #_ctc6845
	    ld bc, #0x88
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x89), a		; data
	    inc hl
	    inc b
	    bit 4,b
	    jr z, ctcloop

   	    ; clear screen
	    ld hl, #0xF800
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0xF802
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
            ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

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
            ld hl, #fd_nmi_handler
            ld (0x0067), hl
	    jp map_kernel
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            out (0xEB), a
            ret

;
;	Swap helpers
;
_hd_xfer_in:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   call map_kernel
	   ret

_hd_xfer_out:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
	   call map_kernel
	   ret

