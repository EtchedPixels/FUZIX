; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl map_for_swap
	.globl platform_interrupt_all
	.globl _platform_reboot
	.globl _platform_monitor
	.globl _bufpool

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
	.globl _ser_type

	; exported debugging tools
	.globl inchar
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
_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this

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
	ld bc,#0x80
	; If writing to 0x80 changes the data we see on an input then
	; it's most likely an SIO and not the 68B50
	out (c),b
	in d,(c)
	inc b
	out (c),b
	in a,(c)
	sub d
;;FIXME	jr z, is_sio
	jr is_sio
	; We have however pooped on the 68B50 setup so put it back into
	; a sensible state.
	ld a,#0x03
	out (c),a
	in a,(c)
	or a
	jr nz, not_acia_either
        ld a, #ACIA_RTS_LOW_A
        out (c),a         		; Initialise ACIA
	ld a,#2
	ld (_ser_type),a
	ret

	;
	; Doomed I say .... doomed, we're all doomed
	;
	; At least until RC2014 grows a nice keyboard/display card!
	;
not_acia_either:
	xor a
	ld (_ser_type),a
	ret

is_sio:	ld a,b
	ld (_ser_type),a

init_partial_uart:

        ld a,#0x00
        out (SIOA_C),a
        ld a,#0x18
        out (SIOA_C),a

        ld a,#0x04
        out (SIOA_C),a
        ld a,#0xC4
        out (SIOA_C),a

        ld a,#0x01
        out (SIOA_C),a
        ld a,#0x18;A?          ; Receive int mode 11, tx int enable (was $18)
        out (SIOA_C),a

        ld a,#0x03
        out (SIOA_C),a
        ld a,#0xE1
        out (SIOA_C),a

        ld a,#0x05
        out (SIOA_C),a
        ld a,#RTS_LOW
        out (SIOA_C),a

        ld a,#0x00
        out (SIOB_C),a
        ld a,#0x18
        out (SIOB_C),a

        ld a,#0x04
        out (SIOB_C),a
        ld a,#0xC4
        out (SIOB_C),a

        ld a,#0x01
        out (SIOB_C),a
        ld a, #0x18;A?          ; Receive int mode 11, tx int enable (was $18)
        out (SIOB_C),a

        ld a,#0x02
        out (SIOB_C),a
        ld a,#SIO_IV		; INTERRUPT VECTOR ADDRESS (needs to go)
        out (SIOB_C),a

        ld a,#0x03
        out (SIOB_C),a
        ld a,#0xE1
        out (SIOB_C),a

        ld a,#0x05
        out (SIOB_C),a
        ld a,#RTS_LOW
        out (SIOB_C),a

        ; ---------------------------------------------------------------------
	; Initialize CTC
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

;=========================================================================
; Kernel code
;=========================================================================
        .area _CODE

_platform_monitor:
	di
	halt
_platform_reboot:
        ; We need to map the ROM back in -- ideally into every page.
        ; This little trick based on a clever suggestion from John Coffman.
	call map_kernel
	rst 0

;=========================================================================
; Common Memory (0xF000 upwards)
;=========================================================================
        .area _COMMONMEM

;=========================================================================

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
	ld a,i
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
	jp po, was_di
	ei
was_di:	pop hl
	pop bc
	ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
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

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
	push af
	xor a
	call rom_control
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
map_save:
	push af
	ld a,(rom_toggle)
	ld (save_rom),a
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

	push af
	ld a,(_ser_type)
	cp #2
	jr z, ocloop_acia

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
	ld a,(_ser_type)
	cp #2
	jr z,inchar_acia
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
