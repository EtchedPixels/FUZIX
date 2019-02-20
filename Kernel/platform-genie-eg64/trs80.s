;
;	    Genie EG64  hardware support
;

            .module genie

            ; exported symbols
            .globl init_early
            .globl interrupt_handler
            .globl _program_vectors
	    .globl platform_interrupt_all

	    .globl go_fast
	    .globl go_slow

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl map_kernel
	    .globl map_process_always

	    .globl _trs80_model
	    .globl _int_disabled
	    .globl _need_resched

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
            .globl outcharhex
	    .globl null_handler

            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0000 upwards after the udata etc)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_int_disabled:
	    .db 1

_need_resched:
	    .db 1

_platform_monitor:
monitor_spin:
	    di
	    jr monitor_spin

platform_interrupt_all:
	    ret

_platform_reboot:
	   di
	   ld sp,#0xffff
	   xor a
	   out (0x43),a
	   rst 0

; -----------------------------------------------------------------------------
; BOOT MEMORY BANK (0x0100)
; -----------------------------------------------------------------------------
            .area _BOOT


init_early:
	    ld a,(4)
	    cp #0x30
	    jr nz, not_m3
	    ld a,#1
	    ld (_trs80_model),a
	    ld a,#0x74
	    out (0xE0),a	; Mask iobus, cassette
	    xor a
	    out (0xE4),a	; and NMI sources
	    jr not_vg
not_m3:
	    ; Detect machine type (Model 1 or LNW80 or VideoGenie ?)
	    ld a,#8
	    out (0xFE),a	; turn off ROM on the LNW80
	    ld hl,#0
	    ld a,(hl)
	    inc (hl)
	    cp (hl)
	    jr z, not_lnw	; if it's RAM it's an LNW80
	    dec (hl)
	    xor a
	    out (0xFE),a	; ROM back on, normal video mode for now
	    ld a,#2		; LNW80
	    ld (_trs80_model), a
	    jr not_vg
not_lnw:
	    ld hl,(0x18F5)
	    ld de,#0x4E53	; 'SN' for VG, 'L3' for TRS80 Model 1
	    or a
	    sbc hl,de
	    jr nz, not_vg
	    ld a,#3
	    ld (_trs80_model),a	; Video Genie
not_vg:
            ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM
;
;	TRS80 speed control. Save anything used except AF
;	The LNW80 does automatic slowing on floppy disk access
;	The Model 3 sprinter type card uses port 95 the same way but
;	also does automatic slow down when needed.
;
;	Only allowed to mess with AF
;
go_slow:
	    ld a,(_trs80_model)
	    or a
	    jr nz, snot_model_1
	    ; A = 0
	    out (254),a
	    ret
snot_model_1:
	    cp #1		; model III ?
	    ret nz
	    xor a
	    out (95),a
	    ret
go_fast:
	    ld a,(_trs80_model)
	    or a
	    jr nz,fnot_model_1
	    ; A = 0
	    inc a
	    out (254),a
	    ret
fnot_model_1:
	    cp #1
	    ret nz
	    ; A = 1
	    out (95),a
	    ret

_program_vectors:
	    ret

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
; FIXME: need to do different things for Video Genie and Model III
outchar:
            ld (0x37E8), a
            ret

;
;	Swap helpers.
;	We have our buffers mapepd in Bank 2 but we don't need to do
;	anything here as we are in common memory and we've carefully
;	arranged that the device driver callers are in BANK2 thus we'll
;	have BANK2 mapped by default, although we may map a user bank
;	in temporarily if going direct to user.
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
	   call nz, map_process_always
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   pop af
	   call nz, map_kernel
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
	   call nz, map_process_always
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
	   pop af
	   call nz, map_kernel
	   ret
