; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_kernel_restore
	.globl map_process
	.globl map_process_save
	.globl map_buffers
	.globl map_kernel_di
	.globl map_process_di
	.globl map_process_always
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl plt_interrupt_all
	.globl _copy_common
	.globl mpgsel_cache
	.globl top_bank
	.globl _kernel_pages
	.globl _plt_reboot
	.globl _bufpool
	.globl _int_disabled

	.globl _qread
	.globl _qwrite

	.globl _code1_end

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl _init_hardware_c
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl interrupt_legacy
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
	.globl _acia_present
	.globl _ctc_port
	.globl _kio_port
	.globl _sio_present
	.globl _sio1_present
	.globl _u16x50_present
	.globl _z180_present
	.globl _eipc_present
	.globl _udata

	; exported debugging tools
	.globl outchar
	.globl inchar

	.globl s__CODE1
	.globl l__CODE1

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

SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_C+1
SIOB_C		.EQU	SIOA_C+2
SIOB_D		.EQU	SIOA_C+3

SIOC_C		.EQU	0x84
SIOC_D		.EQU	SIOC_C+1
SIOD_C		.EQU	SIOC_C+2
SIOD_D		.EQU	SIOC_C+3


; SIO arrangement on the KIO chip
KIOA_C		.EQU	0x89
KIOB_C		.EQU	0x8B
; SIO arrangement on Extreme build KIO at 0xC0
EKIOA_C		.EQU	0xC9
EKIOB_C		.EQU	0xCB

EIPCSA_C	.EQU	0x19
EIPCSB_C	.EQU	0x1B

ACIA_C          .EQU     0x80
ACIA_D          .EQU     0x81
ACIA_RESET      .EQU     0x03
ACIA_RTS_HIGH_A      .EQU     0xD6   ; rts high, xmit interrupt disabled
ACIA_RTS_LOW_A       .EQU     0x96   ; rts low, xmit interrupt disabled
;ACIA_RTS_LOW_A       .EQU     0xB6   ; rts low, xmit interrupt enabled

MAP_BANK1	.equ	0x2221	; 20 is low, 23 is high
BANK1		.equ	0x21
MAP_BANK2	.equ	0x2524	; for now 24 is buffers - FIXME
BANK2		.equ	0x24
MAP_BANK3	.equ	0x2726
BANK3		.equ	0x26


;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD

init_hardware:
        ; program vectors for the kernel
        call do_program_vectors

        ; Stop floppy drive motors
        ld a, #0x0C
        out (FDC_DOR), a

	; Let keyboard initialize freely - float all the lines

	ld a,#0xFF
	out (0xBB),a

	; Check for an EIPC

	in  a,(0xF0)
	and #7
	cp #3
	jr nz, not_eipc
	ld c,#0x10
	call check_ctc

	jr nz, not_eipc

	ld a,#1
	ld (_eipc_present),a

	ld hl,#sio_setup
	ld bc,#0x0A00 + EIPCSA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + EIPCSB_C		; and to SIOB_C
	otir

	jp probe_cpu


not_eipc:
	; Check for a KIO

	ld c,#0x84
	call check_ctc
	jr nz, not_kio80

	ld a,#0x80
	ld (_kio_port),a

	ld hl,#sio_setup
	ld bc,#0xA00 + KIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + KIOB_C		; and to SIOB_C
	otir

	jp probe_cpu

not_kio80:
	ld c,#0xc4
	call check_ctc
	jr nz, not_kioc0

	ld a,#0xC0
	ld (_kio_port),a
	ld hl,#sio_setup
	ld bc,#0xA00 + EKIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + EKIOB_C		; and to SIOB_C
	otir

	; Fall through as the C0 KIO ports are not seem by ROMWBW
	; so are not our console device.
not_kioc0:

	; Play guess the serial port

	;
	; We are booted under ROMWBW, therefore use the same algorithm as
	; ROMWBW so if the probe fails we at least expect it to have failed
	; before we run.
	;
	; FIXME: see if we can cleanly ask ROMWBW for the device type
	;

	;
	; This could be the ACIA control port. If so we mash the settings
	; up but that is ok as we will port them back in the ACIA probe
	;
	; FIXME: ROMWBW order is different ?
	;
	;	Look for an ACIA
	;
	in a,(ACIA_C)			; TX ready should be set by now
	bit 1,a
	jr z, not_acia
	ld a,#ACIA_RESET
	out (ACIA_C),a
	; TX should now have gone
	in a,(ACIA_C)
	bit 1,a
	jr nz, not_acia
	;
	;	Set up the ACIA
	;
	ld a,#2
	out (ACIA_C),a
        ld a, #ACIA_RTS_LOW_A
        out (ACIA_C),a         		; Initialise ACIA
	ld a,#1
	ld (_acia_present),a

not_acia:
	; Look for a 16x50 at 0xA0
	in a,(0xA3)
	ld e,a
	or #0x80		; use the DLAB bit to detect
	ld l,a
	out (0xA3), a
	in a,(0xA1)
	ld d,a			; Remember old speed bits from ROMWBW
	ld a,#0xAA		; Pick 0xAA as it's a valid pattern for baud
	out (0xA1), a		; but not for the control register it overlaps
	in a,(0xA1)
	cp #0xAA
	jr nz, not_16x50
	ld a,e
	out (0xA3),a
	in a,(0xA1)
	cp #0xAA
	jr z, not_16x50
	ld a,l
	out (0xA3),a
	ld a,d			; put the speed back
	out (0xA1),a
	ld a,e
	out (0xA3),a		; and the other settings
	ld a,#1
	ld (_u16x50_present),a

not_16x50:
	ld a,e
	out (0xA3),a

	xor a
	ld c,#SIOA_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jp z, serial_up

	; Repeat the check on SIO B

	xor a
	ld c,#SIOB_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jp z, serial_up

	; Now sanity check the vector register

	ld a,#2
	out (c),a
	ld a,#0xA0
	out (c),a
	ld a,#2
	out (c),a
	in a,(c)
	and #0xF0
	cp #0xA0
	jp nz, serial_up

	ld a,#2
	out (c),a
	ld a,#0x50
	out (c),a
	ld a,#2
	out (c),a
	in a,(c)
	and #0xF0
	cp #0x50
	jr nz, serial_up


;
;	We have an SIO so do the required SIO dance
;
is_sio:	ld a,#1
	ld (_sio_present),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

	xor a
	ld c,#SIOC_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, serial_up

	; Repeat the check on SIO B
	; We have to be careful here because it could be that this is
	; a mirror of the first SIO! Fortunately channel B WR2 can be read
	; as RR2 so we can write the vector to one and check the other, then
	; write a different vector and check again. If they both change
	; it's aliased, if not it really is two interfaces.

	ld a,#2
	out (SIOB_C),a			; vector
	ld a,#0xF0
	out (SIOB_C),a			; vector is now 0xFX
	ld a,#2
	out (SIOD_C),a
	in a,(SIOD_C)			; read it back on the second SIO
	and #0xF0
	cp #0xF0
	; Now put SIOB_C vector back to zero so we don't fight
	; on the bus. Don't touch flags between here and the jr nz
	ld a,#2
	out (SIOB_C),a
	ld a,#0
	out (SIOB_C),a
	jr nz, not_mirrored		; it's not a mirror, might not be an SIO

	; Could be chance or a soft boot
	; Check if the above clear also affected C/D

	ld a,#2
	out (SIOD_C),a
	in a,(SIOD_C)
	and #0xF0
	jr z, serial_up			; It's a mirage

not_mirrored:

	ld a,#2
	out (SIOD_C),a			; Reprogram vector back to zero
	xor a
	out (SIOD_C),a

	ld c,#SIOD_C

	xor a
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, serial_up

	ld a,#0x01
	ld (_sio1_present),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOC_C		; 10 bytes to SIOC_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOD_C		; and to SIOD_C
	otir


serial_up:
	ld c,#0x88
	call check_ctc
probe_cpu:
	;
	; Check CPU type. We might be a Z180 CPU board with standard
	; RC2014 components in which case activate the additional
	; serial ports. If we have no CTC we should use the Z180 timers
	;

	xor a
	dec a
	daa
	cp #0xF9
	jr nz, is_z80

	ld a,#1
	ld (_z180_present),a

is_z80:
	im 1				; set Z80 CPU interrupt mode 1
        jp _init_hardware_c             ; pass control to C, which returns for us

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

check_ctc:
        ; ---------------------------------------------------------------------
	; Initialize CTC
	;
	; Need to do autodetect on this
	;
	; C is the CTC base we want to probe
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
	out (c),a			; set CH0 mode
	inc c
	out (c),a			; set CH1 mode
	inc c
	out (c),a			; set CH2 mode
	inc c
	out (c),a			; set CH3 mode

	;
	; Probe for a CTC
	;

	dec c				; CTC 2

	ld a,#0x47			; CTC 2 as counter
	out (c),a
	ld a,#0xAA			; Set a count
	out (c),a
	in a,(c)

	cp #0xAA
	jr z, ctc_maybe
	cp #0xA9			; Could be one less
	ret nz

ctc_maybe:
	ld a,#0x07
	out (c),a
	ld a,#2
	out (c),a

	; We are now counting down from 2 very fast, so should only see
	; those values on the bus

	ld b,#0
ctc_check:
	in a,(c)
	and #0xFC
	ret nz
	djnz ctc_check

	;
	; Looks like we have a CTC
	;

	ld a,c
	sub a,#2
	ld (_ctc_port),a

	ld a,#0x43	; Turn the CTC off
	out (c),a

	;
	; Set up counter CH3 for SC102 or similar SIO (the SC110 sadly can't be
	; used this way I believe).

	inc c

	ld a,#0x47
	out (c),a
	ld a,#255
	out (c),a

	xor a
	ret

;
;	TTY queues. We keep them in CODE1 so this is as simple as
;	letting the bank logic do the natural thing. As these helpers and
;	tty are in bank 1 it won't even cause us any grief. Common addresses
;	and data addresses also continue to work so we don't need magic for
;	the devinput queue.
;

	.area _CODE1
_qread:
	ld l,(hl)
	ret
_qwrite:
	ld hl,#4
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld a,(hl)
	ld (de),a
	ret

;
;	Return where CODE1 ends. Tricky to do in C as the linker symbols
;	have no leading underscore.
;
_code1_end:
	ld hl,#s__CODE1
	ld de,#l__CODE1
	add hl,de
	ret

;=========================================================================
; Kernel code
;=========================================================================
        .area _COMMONMEM

_plt_reboot:
        ; We need to map the ROM back in -- ideally into every page.
        ; This little trick based on a clever suggestion from John Coffman.
        di
	ld a,#0x0C
	out (FDC_DOR),a			; Floppy off
	xor a
	out (0x99),a
	ld a,#0x81
	out (0x99),a
	; CTC off
	ld a,#0x43
	out (CTC_CH0),a
	out (CTC_CH1),a
	out (CTC_CH2),a
	out (CTC_CH3),a
        ld hl, #(MPGENA << 8) | 0xD3    ; OUT (MPGENA), A
        ld (0xFFFE), hl                 ; put it at the very top of RAM
        xor a                           ; A=0
        jp 0xFFFE                       ; execute it; PC then wraps to 0


;=========================================================================
; Common Memory (0xF000 upwards)
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
	pop bc				; bank
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de
	push bc

	; At this point the common block has already been copied
	call map_process

	call do_program_vectors

	jp map_kernel_restore


do_program_vectors:
	; write zeroes across all vectors
	ld hl,#0
	ld de,#1
	ld bc,#0x007f			; program first 0x80 bytes only
	ld (hl),#0x00
	ldir

	; now install the interrupt vector at 0x0038
	ld a,#0xC3			; JP instruction
	ld (0x0038),a
	ld hl,#interrupt_legacy
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
	ret

;=========================================================================
; Memory management
; - kernel pages:     32 - 34
; - common page:      35
; - user space pages: 36 - 63
;=========================================================================

;=========================================================================
; map_process_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_process_always:
map_process_save:
map_process_always_di:
	push hl
	; We don't need to save the kernel page numbers because we
	; patched them on bank switches
	ld hl,#_udata + U_DATA__U_PAGE
        jr map_process_2_pophl_ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
map_process_di:
	ld a,h
	or l				; HL == 0?
	jr nz,map_process_2		; HL == 0 - map the kernel

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_buffers:
map_kernel_restore:
map_kernel_di:
	push hl
	ld hl,#_kernel_pages
        jr map_process_2_pophl_ret

;=========================================================================
; map_process_2 - map process or kernel pages
; Inputs: page table address in HL
; Outputs: none, HL destroyed
;=========================================================================
map_process_2:
	push de
	push af
	ld de,#mpgsel_cache		; paging registers are write only
					; so cache their content in RAM
	ld a,(hl)			; memory page number for bank #0
	ld (de),a
	out (MPGSEL_0),a		; set bank #0
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank #1
	ld (de),a
	out (MPGSEL_1),a		; set bank #1
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank #2
	ld (de),a
	out (MPGSEL_2),a		; set bank #2
	pop af
	pop de
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push hl
	ld hl,#map_savearea
map_process_2_pophl_ret:
	call map_process_2
	pop hl
	ret

;=========================================================================
; map_save_kernel - save the current page mapping to map_savearea and
; switch to kernel maps. We might switch to any kernel map but that is
; ok as we will make a thunking call from common to the C irq handler
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push hl
	ld hl,(mpgsel_cache)
	ld (map_savearea),hl
	ld hl,(mpgsel_cache+2)
	ld (map_savearea+2),hl
	ld hl,#_kernel_pages
	jr map_process_2_pophl_ret

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: A = page
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
; We use 0x4000-0x7FFF so that all the interrupt stuff is mapped.
;
;=========================================================================
map_for_swap:
	ld (mpgsel_cache + 1),a
	out (MPGSEL_1),a
	ret

	.globl	map_soft81
	.globl	map_soft81_restore
;
;	Magic map functions for the ZX81 emulation page flippery. Only plays
;	with the bottom 32K
;
map_soft81:	; HL is the ptptr and it might be swapped out!
	ld	de,#P_TAB__P_PAGE_OFFSET
	add	hl,de
	ld	a,(hl)
	or	a
	ret	z		; not mapped
	inc	hl
	ld	e,(hl)		; save 2nd byte before we change map
				; and it vanishes
	ld	bc,(mpgsel_cache)
	ld	(soft81_0),bc
	ld	(mpgsel_cache),a
	out	(MPGSEL_0),a
	ld	a,e		; second byte
	ld	(mpgsel_cache + 1),a
	out	(MPGSEL_1),a
	ret

map_soft81_restore:
	ld	hl,(soft81_0)
	ld	(mpgsel_cache),hl
	ld	a,l
	out	(MPGSEL_0),a
	ld	a,h
	out	(MPGSEL_1),a
	ret

soft81_0:
	.byte	0
soft81_1:
	.byte	0

;
;	Bank switch functions
;
	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_0_3
	.globl __bank_1_2
	.globl __bank_1_3
	.globl __bank_2_1
	.globl __bank_2_3
	.globl __bank_3_1
	.globl __bank_3_2

	.globl __stub_0_1
	.globl __stub_0_2
	.globl __stub_0_3
	.globl __stub_1_2
	.globl __stub_1_3
	.globl __stub_2_1
	.globl __stub_2_3
;
;	We are calling into bank 1 from common. We don't know the current
;	bank but we need to restore it as was.
;
__bank_0_1:
	ld bc,#MAP_BANK1		; target pairs always a pair
bank0:
	pop hl				; in linear order
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push hl
	ld a,(_kernel_pages + 1)	; save old page
	ld (_kernel_pages + 1),bc
	ld (mpgsel_cache + 1),bc	; and mark as current
	ld b,a
	ld a,c				; set new page
	out (MPGSEL_1),a
	inc a				; next page follows
	out (MPGSEL_2),a
	ex de,hl
	ld a,b				; old bank
	cp #BANK1			; 1 or 2
	jr z, retbank1
	cp #BANK2
	jr z, retbank2
	call callhl
	ld bc,#MAP_BANK3
banksetbc:
	ld (_kernel_pages + 1), bc
	ld (mpgsel_cache + 1), bc
	ld a,c
	out (MPGSEL_1),a
	ld a,b
	out (MPGSEL_2),a
	ret
retbank1:
	call callhl
	ld bc,#MAP_BANK1
	jr banksetbc
retbank2:
	call callhl
	ld bc,#MAP_BANK2
	jr banksetbc
__bank_0_2:
	ld bc,#MAP_BANK2
	jr bank0
__bank_0_3:
	ld bc,#MAP_BANK3
	jr bank0
;
;	These are easier because we have two banks so know the target and
;	return bank in each case
;
__bank_1_2:
	ld bc,#MAP_BANK2
bank_1_x:
	pop hl				; in linear order
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push hl
	call banksetbc
	ex de,hl
	call callhl
	ld bc,#MAP_BANK1
	jr banksetbc
__bank_1_3:
	ld bc,#MAP_BANK3
	jr bank_1_x

__bank_2_1:
	ld bc,#MAP_BANK1
bank_2_x:
	pop hl				; in linear order
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push hl
	call banksetbc
	ex de,hl
	call callhl
	ld bc,#MAP_BANK2
	jr banksetbc
__bank_2_3:
	ld bc,#MAP_BANK3
	jr bank_2_x

__bank_3_1:
	ld bc,#MAP_BANK1
bank_3_x:
	pop hl				; in linear order
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push hl
	call banksetbc
	ex de,hl
	call callhl
	ld bc,#MAP_BANK3
	jr banksetbc
__bank_3_2:
	ld bc,#MAP_BANK2
	jr bank_3_x

__stub_2_1:
__stub_3_1:
__stub_0_1:
	ld bc,#MAP_BANK1
	jr stub_call
__stub_0_2:
__stub_1_2:
__stub_3_2:
	ld bc,#MAP_BANK2
	jr stub_call
__stub_0_3:
__stub_1_3:
__stub_2_3:
	ld bc,#MAP_BANK3
stub_call:
	pop hl				; return address
	ex (sp),hl			; bank space now target
	ld a,(_kernel_pages+1)		; current bank
	ld (_kernel_pages+1),bc		; target
	ld (mpgsel_cache + 1), bc
	ld b,a
	ld a,c
	out (MPGSEL_1),a
	inc a
	out (MPGSEL_2),a
	ex de,hl
	ld a,b
	cp #BANK1
	jr z, stub_ret_1
	call callhl
	ld bc,#MAP_BANK2
stub_ret:
	ld (_kernel_pages+1),bc
	ld (mpgsel_cache + 1), bc
	ld a,c
	out (MPGSEL_1),a
	inc a
	out (MPGSEL_2),a
	pop bc				; return
	push bc				; correct stack padding
	push bc
	ret				; done
stub_ret_1:
	call callhl
	ld bc,#MAP_BANK1
	jr stub_ret

callhl:	jp (hl)
;
;	Helpers for swap
;

_copy_common:
	pop bc
	pop hl
	pop de
	push de
	push hl
	push bc
	ld a,e
	call map_for_swap
	ld hl,#0xF200
	ld de,#0x7200
	ld bc,#0x0E00
	ldir
	jp map_kernel


; MPGSEL registers are read only, so their content is cached here
mpgsel_cache:
	.db	0,0,0
top_bank:	; the shared tricks code needs this name for cache+3
	.db	0

; kernel page mapping
_kernel_pages:
	.db	32,33,34,35

; memory page mapping save area for map_save/map_restore
map_savearea:
	.db	0,0,0,0

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

;	We need acia_present in common space because we might call these
;	helpers from common code

_acia_present:
	.byte 0

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:

	push af
	ld a, (_acia_present)
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
        out (VFD_D),a
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


;
;	And the Z180 support code
;

	.include "../lib/z180-support.s"
