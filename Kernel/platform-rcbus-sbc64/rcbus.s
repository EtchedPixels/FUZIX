; 2014-02-19 Sergey Kiselev
; RCBUS hardware specific code

        .module rcbus

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_proc_a
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
	.globl _plt_suspend
	.globl _bufpool
	.globl _int_disabled
	.globl _cpld_bitbang
	.globl _kbank

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl _suspend
	.globl _ctc_present
	.globl _sio_present
	.globl _sio1_present
	.globl _tty_resume
	.globl _ide_resume
	.globl _udata
	.globl _mach_zrcc
	.globl ___sdcc_enter_ix

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../kernel-z80.def"

;=========================================================================
; Constants
;=========================================================================
CONSOLE_DIVISOR		.equ	(1843200 / (16 * CONSOLE_RATE))
CONSOLE_DIVISOR_HIGH	.equ	(CONSOLE_DIVISOR >> 8)
CONSOLE_DIVISOR_LOW	.equ	(CONSOLE_DIVISOR & 0xFF)

RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

; Base address of SIO/2 chip 0x80
; For the Scott Baker SIO card adjust the order to match rcbus.h

SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_C+1
SIOB_C		.EQU	SIOA_C+2
SIOB_D		.EQU	SIOA_C+3

SIOC_C		.EQU	0x84
SIOC_D		.EQU	SIOC_C+1
SIOD_C		.EQU	SIOC_C+2
SIOD_D		.EQU	SIOC_C+3

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
	ld hl,#128
	ld (_ramsize), hl
	ld hl,#64
	ld (_procmem), hl

	; We are in the upper bank so we can do an aliasing check to figure
	; out the board

	; On the Z80SBC64 this makes our low 32K match our top 32K. On the
	; ZRCC it makes our low 32K point into the kernel
	ld a,#0x01
	out (0x1f),a
	ld hl,#_kbank
	ld de,#_kbank-0x8000
	ld a,(de)
	cp (hl)
	jr nz, is_zrcc			; not aliased
	; Is it a false aliasing ?
	inc (hl)
	ld a,(de)
	cp (hl)				; if not aliased match will break
	jr nz, is_zrcc_fix
	; Restore the kbank
	dec (hl)
	; We are on a Z80SBC64
	ld a,#3
	ld (_kbank),a
	; Put the kernel back
	out (0x1f),a
	jr vectors
is_zrcc_fix:
	dec (hl)			; undo the byte we beat up
is_zrcc:
	ld a,#1
	ld (_kbank),a			; Change kernel bank to 1
	; Fall through into the setup
vectors:
	call program_kvectors

	; Install RST helpers
	ld hl,#rstblock
	ld de,#8
	ld bc,#32
	ldir

	; Look for an SIO using the ROMWBW algorithm

	xor a
	ld c,#SIOA_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, no_sio0

	; Repeat the check on SIO B

	xor a
	ld c,#SIOB_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, no_sio0

	ld a,#0x01
	ld (_sio_present),a

no_sio0:

	xor a
	ld c,#SIOC_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, no_sio1

	; Repeat the check on SIO B

	xor a
	ld c,#SIOD_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, no_sio1

	ld a,#0x01
	ld (_sio1_present),a

no_sio1:

resume_hardware:

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir
	ld hl,#sio_setup
	ld bc,#0xA00 + SIOC_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOD_C		; and to SIOB_C
	otir

serial_up:
        ; ---------------------------------------------------------------------
	; Initialize CTC
	;
	; Need to do autodetect on this
	;
	; We must initialize all channels of the CTC. The documentation
	; states that the initial CTC state is undefined and we don't want
	; random interrupt surprises
	;
	; ---------------------------------------------------------------------

	;
	; Defense in depth - shut everything up first
	;

	ld a,#0x43
	out (CTC_CH0),a			; set CH0 mode
	out (CTC_CH1),a			; set CH1 mode
	out (CTC_CH2),a			; set CH2 mode
	out (CTC_CH3),a			; set CH3 mode

	;
	; Probe for a CTC
	;

	ld a,#0x47			; CTC 2 as counter
	out (CTC_CH2),a
	ld a,#0xAA			; Set a count
	out (CTC_CH2),a
	in a,(CTC_CH2)
	cp #0xAA			; Should not have changed
	jr z, maybe_ctc
	cp #0xA9			; But might by one
	jr nz, no_ctc
maybe_ctc:
	ld a,#0x07
	out (CTC_CH2),a
	ld a,#2
	out (CTC_CH2),a

	; We are now counting down from 2 very fast, so should only see
	; those values on the bus

	ld b,#0
ctc_check:
	in a,(CTC_CH2)
	and #0xFC
	jr nz, no_ctc
	djnz ctc_check

	;
	; Looks like we have a CTC
	;

have_ctc:
	ld a,#1
	ld (_ctc_present),a

	;
	; Set up timer for 200Hz
	;

	ld a,#0xB5
	out (CTC_CH2),a
	ld a,#144
	out (CTC_CH2),a	; 200 Hz

	;
	; Set up counter CH3 for official SIO (the SC110 sadly can't be
	; used this way).

	ld a,#0x47
	out (CTC_CH3),a
	ld a,#255
	out (CTC_CH3),a

no_ctc:
        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	im 1				; set Z80 CPU interrupt mode 1
	ret

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x18
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte RTS_LOW

        .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	ld a,#0x03
	out (0x1f),a
	rst 0

_plt_suspend:
	; No suspend on ZRCC only on SBC/MBC64
	ld a,(_mach_zrcc)
	or a
	ret nz
	call _suspend
	; Re-initialize the CTC and basic SIO setup
	call resume_hardware
	; Restore the live SIO configuration
	call _tty_resume
	; Now fix the IDE controller
	; We can't use the generic code for probing and setup because we
	; discarded it at boot!
	call _ide_resume
	ret

_kbank:
	.db 3				; for Z80SBC64, will be on for ZRCC
_int_disabled:
	.db 1
pagereg:
	.db 0
pagesave:
	.db 0

plt_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_proc

	; write zeroes across all vectors
	ld hl,#0
	ld de,#1
	ld bc,#0x007f			; program first 0x80 bytes only
	ld (hl),#0x00
	ldir

program_kvectors:
	; This is ok as for the bank 3 it's already JP

	ld a,#0xC3			; JP instruction
	ld (0x0000),a			; Must be present for NULL checker

	; now install the interrupt vector at 0x0038
	ld (0x0038),a
	ld hl,#interrupt_handler
	ld (0x0039),hl

	; set restart vector for UZI system calls
	ld (0x0030),a			; rst 30h is unix function call vector
	ld hl,#unix_syscall_entry
	ld (0x0031),hl

	; We can't add a NULL handler - it's the restart for power off

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; Memory management
;=========================================================================

map_proc:
map_proc_di:
	ld a,h
	or l				; HL == 0?
	jr z,map_kernel			; HL == 0 - map the kernel
	ld a,(hl)
map_for_swap:
map_proc_a:
	ld (pagereg),a
	out (0x1f),a
	ret

map_proc_always:
map_proc_always_di:
	push af
	ld a,(_udata + U_DATA__U_PAGE)
map_pop_a:
	ld (pagereg),a
	out (0x1f),a
	pop af
	ret

map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push af
	ld a,(_kbank)
	jr map_pop_a

map_restore:
	push af
	ld a,(pagesave)
	jr map_pop_a

map_save_kernel:
	push af
	ld a,(pagereg)
	ld (pagesave),a
	ld a,(_kbank)
	jr map_pop_a

;
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld b,#0x06
	ld c,l
	ld hl,#_sio_r
	otir
	ret

; C entry point
_cpld_bitbang:
	ld a,l
; Debug entry point
;
; Based on the ROM code but slightly tighter
; - use ld a,#0 so 0 and 1 bits are same length
; - don't duplicate excess code in the hi/lo bit paths
; - use conditional calls to keep 0/1 timing identical
;
; FIXME: my math says it's still slightly off timing.
;
outchar:
	push bc
	ld c,a
	ld b,#8
	call lobit
	ld a,c
txbit:
	rrca
	call c, hibit
	call nc, lobit
	djnz txbit
	pop bc
hibit:
	push af
	ld a,#0xff
	out (0xf9),a
	ld a,#7
bitwait:
	dec a
	jp nz,bitwait
	pop af
	ret
lobit:
	push af
	ld a,#0
	out (0xf9),a
	ld a,#7
	dec a
	jp bitwait

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
