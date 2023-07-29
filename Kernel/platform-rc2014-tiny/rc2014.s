; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_process_di
	.globl map_process_always_di
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
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
	.globl _acia_present
	.globl _ctc_present
	.globl _sio_present

	; exported debugging tools
	.globl inchar
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
; For the Scott Baker SIO card adjust the order to match rc2014.h
SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_C+1
SIOB_C		.EQU	SIOA_C+2
SIOB_D		.EQU	SIOA_C+3

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
	ld hl,#64
	ld (_ramsize), hl
	ld hl,#48
	ld (_procmem), hl

	; Play guess the serial port

	;
	; This could be the ACIA control port. If so we mash the settings
	; up but that is ok as we will put them back in the SIO probe
	;

try_acia:
	;
	;	Look for an ACIA
	;
	in a,(ACIA_C)
	bit 1,a
	jr z, not_acia
	ld a,#ACIA_RESET
	out (ACIA_C),a
	; TX should now have gone
	in a,(ACIA_C)
	bit 1,a
	jr nz, not_acia
	;	Set up the ACIA
        ld a, #ACIA_RTS_LOW_A
        out (ACIA_C),a         		; Initialise ACIA
	ld a,#1
	ld (_acia_present),a
	jp serial_up

not_acia:
	xor a
	ld c,#SIOA_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, not_sio_either

	; Repeat the check on SIO B

	xor a
	ld c,#SIOB_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr nz, is_sio


	;
	; Doomed I say .... doomed, we're all doomed
	;
	; At least until RC2014 grows a nice keyboard/display card!
	;
not_sio_either:
	; Fall through and hope
;
;	We have an SIO so do the required SIO hdance
;
is_sio:	ld a,#1
	ld (_sio_present),a

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
	cp #0xAA
	jr z, maybe_ctc
	cp #0xA9			; Should be one less
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

;=========================================================================
; Kernel code
;=========================================================================
        .area _CODE

_plt_monitor:
	di
	halt
_plt_reboot:
	; Silence the CTC - ROMWBW blows up if the CTC is left running
	ld a,#0x43
	out (CTC_CH0),a
	out (CTC_CH1),a
	out (CTC_CH2),a
	out (CTC_CH3),a
	call map_kernel
	rst 0

;=========================================================================
; Common Memory (0xC000 upwards)
;=========================================================================
        .area _COMMONMEM

;=========================================================================

_int_disabled:
	.db 1

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

	ld (0x0000),a
	ld hl,#null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; Memory management
;=========================================================================

;
;	The ROM is a toggle. That makes it exciting if we get it wrong!
;
;	We *must* have interrupts off here to avoid double toggles.
;

rom_toggle:
	.byte 0		;	ROM starts mapped
save_rom:
	.byte 0		;	For map_save/restore

;
;	Centralize all control of the toggle in one place so we can debug it
;
rom_control:
	push bc			; Messy - clean me up!
	ld c,a
	push hl
	ld a,(_int_disabled)
	push af
	ld a,c
	di
	ld hl,#rom_toggle
	cp (hl)
	jr z, no_work
	ld (hl),a
	out (0x38),a	; 	anything toggles
no_work:
	pop af
	or a
	jr nz, was_di
	ei
was_di:	pop hl
	pop bc
	ret

rom_control_di:		;	Interrupts known to be off, int_disabled may
			;	be inaccurate
	push hl
	ld hl, #rom_toggle
	cp (hl)
	jr z, no_work_di
	ld (hl),a
	out (0x38),a
no_work_di:
	pop hl
	ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
map_process_di:
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
; map_process_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_process_always:
	push af
	ld a,#1
	call rom_control
	pop af
	ret

map_process_always_di:
	push af
	ld a,#1
	call rom_control_di
	pop af
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
	xor a
	call rom_control
	pop af
	ret

map_kernel_di:
	push af
	xor a
	call rom_control_di
	pop af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push af
	ld a,(save_rom)
	call rom_control
	pop af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push af
	; This one has a quirk. Because we may be switching to the kernel
	; bank the int_disable flag is not reliable.
	ld a,(rom_toggle)
	ld (save_rom),a
	xor a
	call rom_control_di
	pop af
	ret
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


;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:

	push af
	ld a,(_acia_present)
	or a
	jr nz, ocloop_acia

	; wait for transmitter to be idle
ocloop_sio:
        xor a                   ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)		; read Line Status Register
	and #0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (SIOA_D),a
	jr out_done

	; wait for transmitter to be idle
ocloop_acia:
	in a,(ACIA_C)		; read Line Status Register
	and #0x02			; get THRE bit
	jr z,ocloop_acia
	; now output the char to serial port
	pop af
	out (ACIA_D),a
out_done:
	ret

;=========================================================================
; inchar - Wait for character on UART, return in A
; Inputs: none
; Outputs: A - received character, F destroyed
;=========================================================================
inchar:
	ld a,(_acia_present)
	or a
	jr nz,inchar_acia
inchar_s:
        xor a                           ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar_s			; no data, wait
	in a,(SIOA_D)   		; read the character from the UART
	ret
inchar_acia:
	in a,(ACIA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar_acia		; no data, wait
	in a,(ACIA_D)   		; read the character from the UART
	ret
