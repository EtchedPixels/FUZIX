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
	.globl map_proc_always
	.globl map_io

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
        .include "../../cpu-z80/kernel-z80.def"

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
;	Read I/O memory'
;
	.globl _ioread

_ioread:
	call map_io
	ld l,(hl)
	jp map_kernel

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
	call nz, map_proc_always
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
	call nz, map_proc_always
	ld bc, #0xC8			; 256 bytes to 0xC8
	otir
	pop af
	ret z
	jp map_kernel

;
;	Keyboard
;
	.globl _keyscan
	.globl _anykey
	.globl _keyin

_keyscan:
	call map_io
	ld b,#8
	ld hl,#0x3801
	ld de,#_keyin
scank:
	ld a,(hl)
	ld (de),a
	inc de
	sla l
	djnz scank
	jp map_kernel

_anykey:
	call map_io
	ld a,(0x38FF)
	ld l,a
	jp map_kernel

;
;	Display
;
videopos:
	; E = Y D = X, output HL = addr, C preserved
	ld a,e
	add a	;	x 2 (max 30)
	add a	;	x 4 (max 60)
	add a	;	x 8 (max 120)
	add a	;	x 16 (max 240)
	ld l,a
	ld h,#0
	add hl,hl  ;	x 32
	add hl,hl  ;	x 64
	ld e,d
	ld d,#0
	add hl,de	; Now holds the offset
	ld de,#0x3C00
	add hl,de	; and screen mapped address
	jp map_io	; map screen

	.globl _vt_check_lower
	.globl _plot_char
	.globl _scroll_up
	.globl _scroll_down
	.globl _clear_lines
	.globl _clear_across
	.globl _cursor_on
	.globl _cursor_off
	.globl _cursor_disable
	.globl _vtattr_notify

	.globl _video_lower

_vt_check_lower:
	call map_io
	ld hl,#0x3C00
	ld a,#'a';
	ld (hl),a
	ld l,(hl)
	call map_kernel
	cp l
	ret nz		; No lower
	ld a,#1
	ld (_video_lower),a
	ret

_plot_char:
	pop hl
	pop de
	pop bc
	push bc
	push de
	push hl		; D = X E = Y C = char
	call videopos
	ld a,(_video_lower)
	or a
	ld a,c
	jr nz, has_lower
	cp #96
	jr c, no_modify
	bit 7,a
	jr nz, no_modify
	sub #32
no_modify:
has_lower:
	ld (hl),a
	jp map_kernel

_scroll_down:
	ld hl,#0x3FBF	; last line
	ld de,#0x3FFF	; line above
	ld bc,#0x03C0	; all but one line
	call map_io
	lddr
	jp map_kernel

_scroll_up:
	ld hl,#0x3C40
	ld de,#0x3C00
	ld bc,#0x03C0
	call map_io
	ldir
	jp map_kernel

_clear_lines:
	pop hl
	pop de
	push de
	push hl		; E = lines D = count
	xor a
	cp d		; no work ?
	ret z
	ld c,d		; save
	ld d,a		; clear d
	call videopos
	ld a,#0x20
wipeo:
	ld b,#0x40
wipe1:
	ld (hl),a
	inc hl
	djnz wipe1
	dec c
	jr nz,wipeo
	jp map_kernel

_clear_across:
	pop hl
	pop de
	pop bc
	push bc
	push de
	push hl		; DE = coords, C = count
	xor a
	cp c
	ret z		; No work
	call videopos
	ld b,c
	ld a,#0x20
wipea:
	ld (hl),a
	inc hl
	djnz wipea
	jp map_kernel

_cursor_on:
	pop hl
	pop de
	push de
	push hl
	call videopos
	ld a,(hl)
	ld (cursorpos),hl
	ld (cursorpeek),a
	ld (hl),#0x8F
	jp map_kernel

_cursor_off:
	call map_io
	ld hl,(cursorpos)
	ld a,(cursorpeek)
	ld (hl),a
	jp map_kernel

_vtattr_notify:
_cursor_disable:
	ret

cursorpos:
	.dw	0
cursorpeek:
	.db	0
_video_lower:
	.db	0
