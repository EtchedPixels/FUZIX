;
;	    Genie EG64  hardware support
;

        .module genie

        ; exported symbols
        .globl init_early
        .globl interrupt_handler
        .globl _program_vectors
	.globl plt_interrupt_all

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
        .globl _plt_monitor
        .globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl istack_top
        .globl istack_switched_sp
        .globl outcharhex
	.globl null_handler
	.globl _vtoutput

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0000 upwards after the udata etc)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_int_disabled:
	.db 1

_need_resched:
	.db 0

_plt_monitor:
monitor_spin:
	di
	jr monitor_spin

plt_interrupt_all:
	ret


; -----------------------------------------------------------------------------
; Needs to be in 4000-7FFF
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_plt_reboot:
	di
	ld sp,#0
	xor a
	out (0x43),a
	out (0xC0),a
	rst 0

init_early:
	ld bc,#0xC0
	out (c),b
	ld a,(4)
	out (c),c
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
	out (c),b
	ld hl,(0x18F5)
	out (c),c
	ld de,#0x4E53	; 'SN' for VG, 'L3' for TRS80 Model 1
	or a
	sbc hl,de
	jr nz, not_vg
	ld a,#3
	ld (_trs80_model),a	; Video Genie
not_vg:
	ld bc,#0xC0C0
	out (c),b		; Back to RAM mapping
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
	ret z
	; A = 0
	out (254),a
	ret
go_fast:
	ld a,(_trs80_model)
	or a
	ret z
	; A = 0
	inc a
	out (254),a
	ret

_program_vectors:
	ret

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	push bc
	push de
	push hl
	call map_kernel
	ld (outtmp),a
	ld hl,#1
	push hl
	ld hl,#outtmp
	push hl
	call _vtoutput
	pop hl
	pop hl
	pop hl
	pop de
	pop bc
	ret

outtmp:	.db 0
;
;	Disk transfer helpers.
;
_hd_xfer_in:
	pop de
	pop hl
	push hl
	push de
	ld a, (_hd_page)
	or a
	push af
	call nz, map_process_always
	ld bc, #0xC8			; 256 bytes from 0xC8
	inir
	pop af
	ret z
	jp map_kernel

_hd_xfer_out:
	pop de
	pop hl
	push hl
	push de
	ld a, (_hd_page)
	or a
	push af
	call nz, map_process_always
	ld bc, #0xC8			; 256 bytes to 0xC8
	otir
	pop af
	ret z
	jp map_kernel
