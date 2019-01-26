; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_process_a
	.globl map_kernel_di
	.globl map_process_di
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl map_buffers
	.globl platform_interrupt_all
	.globl _platform_reboot
	.globl _platform_monitor
	.globl _bufpool
	.globl _int_disabled
	.globl _cpld_bitbang

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../kernel.def"

;=========================================================================
; Constants
;=========================================================================
CONSOLE_DIVISOR		.equ	(1843200 / (16 * CONSOLE_RATE))
CONSOLE_DIVISOR_HIGH	.equ	(CONSOLE_DIVISOR >> 8)
CONSOLE_DIVISOR_LOW	.equ	(CONSOLE_DIVISOR & 0xFF)

RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

; Base address of SIO/2 chip 0x80
; For the Scott Baker SIO card adjust the order to match rc2014.h
SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_D+1
SIOB_C		.EQU	SIOA_D+2
SIOB_D		.EQU	SIOA_D+3

ACIA_C          .EQU     0x80
ACIA_D          .EQU     0x81
ACIA_RESET      .EQU     0x03
ACIA_RTS_HIGH_A      .EQU     0xD6   ; rts high, xmit interrupt disabled
ACIA_RTS_LOW_A       .EQU     0x96   ; rts low, xmit interrupt disabled
;ACIA_RTS_LOW_A       .EQU     0xB6   ; rts low, xmit interrupt enabled

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

	; FIXME: autodetect SIO

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
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
	; ---------------------------------------------------------------------

	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH0),a			; set CH0 mode
	ld a,#0				; time constant = 256
	out (CTC_CH0),a			; set CH0 time constant
	ld a,#0x57			; counter mode, FIXME C7 enable interrupts
	out (CTC_CH1),a			; set CH1 mode
	ld a,#180			; time constant = 180
	out (CTC_CH1),a			; set CH1 time constant
	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH2),a			; set CH2 mode
	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH3),a			; set CH3 mode

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

_platform_monitor:
_platform_reboot:
	ld a,#0x03
	out (0x1f),a
	rst 0

_int_disabled:
	.db 1
pagereg:
	.db 0
pagesave:
	.db 0

platform_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_process

	; write zeroes across all vectors
	ld hl,#0
	ld de,#1
	ld bc,#0x007f			; program first 0x80 bytes only
	ld (hl),#0x00
	ldir

	; now install the interrupt vector at 0x0038
	ld a,#0xC3			; JP instruction
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

map_process:
map_process_di:
	ld a,h
	or l				; HL == 0?
	jr z,map_kernel			; HL == 0 - map the kernel
	ld a,(hl)
map_for_swap:
map_process_a:
	ld (pagereg),a
	out (0x1f),a
	ret

map_process_always:
map_process_always_di:
	push af
	ld a,(U_DATA__U_PAGE)
map_pop_a:
	ld (pagereg),a
	out (0x1f),a
	pop af
	ret

map_buffers:
map_kernel:
map_kernel_di:
	push af
	ld a,#3
	jr map_pop_a

map_restore:
	push af
	ld a,(pagesave)
	jr map_pop_a

map_save_kernel:
	push af
	ld a,(pagereg)
	ld (pagesave),a
	ld a,#3
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

