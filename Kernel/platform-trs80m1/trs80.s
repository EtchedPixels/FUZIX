;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl interrupt_handler
            .globl _program_vectors
	    .globl platform_interrupt_all
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl map_kernel_restore

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl _bufpool
	    .globl bufend

	    .globl _trs80_model

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
	    jr _platform_monitor

platform_interrupt_all:
	    ret

_platform_reboot:
	   di
	   ld sp,#0xffff
	   xor a
	   out (0x43),a
	   rst 0

; -----------------------------------------------------------------------------
; BOOT MEMORY BANK (below 0x8000)
; -----------------------------------------------------------------------------
            .area _BOOT


init_early:
	    ; Detect machine type (Model 1 or LNW80 or VideoGenie ?)
	    ld a,#8
	    out (0xFE),a	; turn off ROM on the LNW80
	    ld hl,#0
	    ld a,(hl)
	    inc (hl)
	    cp (hl)
	    jr z, not_lnw
	    dec (hl)
	    xor a
	    out (0xFE),a	; ROM back on, normal video mode for now
	    ld a,#2		; LWN80
	    ld (_trs80_model), a
not_lnw:
	    ld hl,(0x18F5)
	    ld de,#0x4E53	; 'SN' for VG, 'L3' for TRS80 Model 1
	    or a
	    sbc hl,de
	    jr nz, not_vg
	    ld a,#3
	    ld (_trs80_model),a	; Video Genie
not_vg:
	    call _rom_vectors
            ret

	    .area _DATA
	    .byte 0x01		; Default model is TRS80 model 1

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
	   pop bc
	   pop hl
	   push hl
	   push bc
	   push de
	   ld a, (_hd_page)
	   or a
	   push af
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   pop af
	   call nz, map_kernel_restore
	   ret

_hd_xfer_out:
	   pop de
	   pop bc
	   pop hl
	   push hl
	   push bc
	   push de
	   ld a, (_hd_page)
	   or a
	   push af
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
	   pop af
	   call nz, map_kernel_restore
	   ret

