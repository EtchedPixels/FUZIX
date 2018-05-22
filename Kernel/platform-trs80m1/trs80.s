;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl platform_interrupt_all

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl outcharhex
	    .globl null_handler
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl _bufpool
	    .globl bufend

            .include "kernel.def"
            .include "../kernel.def"

	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS
bufend:
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_platform_monitor:
	    di
	    halt

platform_interrupt_all:
	    ret

_platform_reboot:
	   di
	   ld sp,#0xffff
	   xor a
	   out (0x43),a
	   rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0x8000)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_program_vectors:
	    ret

_rom_vectors:
	    ld a,#0xC3
	    ld (0x4012), a
            ld hl, #interrupt_handler
            ld (0x4013), hl

            ; set restart vector for UZI system calls
            ld (0x400F), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x4010), hl

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

