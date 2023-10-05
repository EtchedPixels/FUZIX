;
;	Minimal Z1013 support
;
        .module z1013

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_proc_di
	.globl map_proc_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl map_buffers
	.globl plt_interrupt_all
	.globl _plt_reboot
	.globl _plt_monitor
	.globl _bufpool
	.globl _int_disabled

        ; imported symbols
	.globl init
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl null_handler
	.globl _video_init

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

;=========================================================================
; Buffers
;=========================================================================
        .area _BUFFERS
	.globl kernel_endmark

_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this
;
;	So we can check for overflow
;
kernel_endmark:

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
	ld	hl,#64
	ld	(_ramsize), hl
	ld	hl,#35
	ld	(_procmem), hl
	ld	hl,#null_handler
	ld	(1),hl
	ld	a,#0xC3
	ld	(0),a
	jp	_video_init

;=========================================================================
; Common Memory (mapped low below ROM)
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
	in	a,(4)			; ensure the monitor is mapped
	and	a,#0xEF
	out	(4),a
	rst	0x38			; To monitor
	.word	0x01			; Wait for key
_plt_reboot:
	call	map_kernel		; ROM in
	jp	init

;=========================================================================

_int_disabled:
	.db 1

; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
	ret

;=========================================================================
; Memory management
;=========================================================================

;
	.globl rom_state	; for debug

rom_state:
	.byte 1		;	ROM starts mapped
save_rom:
	.byte 0		;	For map_save/restore

;
;	Centralize all control of the toggle in one place so we can debug it
;
rom_in:
	in	a,(4)
	set	5,a
	out	(4),a
	ld	(rom_state),a
	ret

rom_out:
	in	a,(4)
	res	5,a
	out	(4),a
	xor	a
	ld	(rom_state),a
	ret


;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
	ld a,h
	or l				; HL == 0?
	jr z,map_kernel			; HL == 0 - map the kernel

	; fall through

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
;=========================================================================
map_for_swap:

	; fall through

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
	di
	push af
	call rom_out
;
;	Restore interrupt status based upon the flag (called with interrupts
;	disabled). AF is currently stacked
;
irqfix:
	ld	a,(_int_disabled)
	or	a
	jr	nz, still_di
	ei
still_di:
	pop	af
	ret

map_proc_always_di:
	push	af
was_u:
	call	rom_out
	pop	af
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
map_kernel:
map_kernel_restore:
	push af
	di
	call rom_in
	jr   irqfix

map_kernel_di:
	push	af
was_k:
	call	rom_in
	pop	af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push af
	ld a,(save_rom)
	or a
	jr nz, was_k
	jr was_u

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push af
	ld a,(rom_state)
	ld (save_rom),a
	call rom_in
	pop af
	ret


;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	push hl
	ld hl,(outpt)
	ld (hl),a
	inc hl
	ld (outpt),hl
	pop hl
	ret
outpt:	.word 0xEC00

;
;	The second set of very performance sensitive routines are accesses
;	to user space. We thus provide our own modified versions of these
;	for speed
;
;	We put all kernel writable space and kernel data/const into the low
;	block (0x0000-0x1FFF), so that when user space is mapped all
;	valid kernel addresses are reachable directly.
;

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_proc_always
	.globl  map_kernel
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
uputget:
        ; load DE with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
	ret	; 	Z is still false

__uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	call map_proc_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
	call map_proc_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
uput_out:
	call map_kernel
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
	jr uput_out

;
__uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl
	push de
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_always
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out


	.area _COMMONMEM
;
;	Character pending or 0. These routines use the ROM so we need to be
;	careful they don't re-enter. keycheck is always called with
;	interrupts off.
;
	.globl _keycheck
;
_keycheck:
	in a,(4)	;	ROM in if paged out
	and #0xEF
	out (4),a
	push ix
	push iy
	rst 0x20
	.byte 4
	pop iy
	pop ix
	ld l,a
	in a,(4)	;	Reverse the ROM page if supported
	or #0x10
	out (4),a
	ret

;
;	Beeper (if there is a jump table)
;
	.globl _do_beep

_do_beep:
	di
	push	ix
	push	iy
	in	a,(4)	;	ROM in
	and	#0xEF
	out	(4),a
	call	do_beep
	in	a,(4)
	or	#0x10
	out	(4),a
	pop	iy
	pop	ix
	ld	a,(_int_disabled)
	or	a
	ret	nz
	ei
	ret

do_beep:
	ld	hl,#0xFFD6
	ld	de,#0x03
	ld	a,#0xC3
	cp	(hl)
	ret	nz
	add	hl,de
	cp	(hl)
	ret	nz
	add	hl,de
	cp	(hl)
	ret	nz
	; Looks like a valid jump table
	jp	(hl)

	.globl _rd_dptr
	.globl _rd_page
	.globl _rd_block

_rd_dptr:
	.word	0
_rd_page:
	.byte	0
_rd_block:
	.word	0

ramconf5:
	ld 	hl,(_rd_block)	; Requested 512 byte block
	add	hl,hl		; Turn HL into 256 byte blocks
	ld	a,l
	out	(0x5e),a	; Set the middle byte
	xor	a
	out	(0x5f),a	; Clear the counter
	ld	a, #0x58	; Work out which port to use
ramconf:
	add 	a,h
	ld	c,a		; port
	ld	a,(_rd_page)	; check if we need to page user space in
	or	a
	call	nz, map_proc_always
	ld	b,#0		; Set up b for the caller
	ld	a,l
	inc	a		; second port info
	ld	hl,(_rd_dptr)	; Set up HL for the caller
	ret

ramconf9:
	ld 	hl,(_rd_block)	; Requested 512 byte block
	add	hl,hl		; Turn HL into 256 byte blocks
	ld	a,l
	out	(0x9e),a	; Set the middle byte
	xor	a
	out	(0x9f),a	; Clear the counter
	ld	a, #0x98	; Work out which port to use
	jr	ramconf

	.globl	_ramread5

_ramread5:
	call	ramconf5
	inir
	out	(0x5e),a
	inir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.globl	_ramwrite5

_ramwrite5:
	call	ramconf5
	otir
	out	(0x5e),a
	otir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.globl	_ramread9

_ramread9:
	call	ramconf9
	inir
	out	(0x9e),a
	inir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.globl	_ramwrite9

_ramwrite9:
	call	ramconf9
	otir
	out	(0x9e),a
	otir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.area _DISCARD

	.globl	_ramdet5

_ramdet5:
	xor	a		;	Point to start of ram disc
	ld	l,a		;	Clear return
	out	(0x5f),a	;	Clear counter
	in	a, (0x5f)	;	Should read back as 0
	or	a
	ret	nz
	in	a,(0x58)	;	Read a byte
	in	a,(0x5f)	;	Counter now reads back as 1
	dec	a
	ret	nz
	inc	l
	ret

	.globl	_ramdet9

_ramdet9:
	xor	a		;	Point to start of ram disc
	ld	l,a		;	Clear return
	out	(0x9f),a	;	Clear counter
	in	a, (0x9f)	;	Should read back as 0
	or	a
	ret	nz
	in	a,(0x98)	;	Read a byte
	in	a,(0x9f)	;	Counter now reads back as 1
	dec	a
	ret	nz
	ld	l,#2
	ret

;
;	Hooks for PIO provided timer tickery. We never have both CTC
;	and PIO enabled.
;
	.area _COMMONMEM

	.globl pio0_intr

;
;	This is level triggered so we need to do a bit of work
;
pio_polarity:
	.byte	0
;
;	The PIO interrupts on low or high state. We do a little bit of
;	stage management to instead use it to spot one of the edges of
;	the square wave input
;
pio0_intr:
	push	af
	ld	a,(pio_polarity)
	or	a
	; low to high or high to low ?
	jr	nz, hilo
	inc	a
	; low to high edge - timer interrupt
	ld	(pio_polarity),a
	; Set the next interrupt to be high so we don't keep firing
	ld	a,#0xB7
	out	(0x02),a
	ld	a,#0x08
	out	(0x02),a
	pop	af
	; We saw a timer tick edge for our 10Hz clock (will do the reti for
	; us)
	jp	interrupt_handler

hilo:
	xor	a
	ld	(pio_polarity),a
	; Set the next interrupt to be low
	ld	a,#0x97
	out	(0x02),a
	ld	a,#0x08
	out	(0x02),a
	pop	af
	reti
