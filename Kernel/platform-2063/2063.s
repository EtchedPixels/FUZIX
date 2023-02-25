;
;	2063 specific hardware implementation/ Mostly
;	memory banking
;

        .module z80_2063

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_process_a
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
	.globl _gpio

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl _udata

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../kernel-z80.def"

;=========================================================================
; Constants
;=========================================================================
RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

; Base address of SIO/2 chip 0x80
; For the Scott Baker SIO card adjust the order to match rcbus.h

SIOA_C		.EQU	0x32
SIOA_D		.EQU	0x30
SIOB_C		.EQU	0x33
SIOB_D		.EQU	0x31

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
	ld hl,#512
	ld (_ramsize), hl
	ld hl,#512-64
	ld (_procmem), hl
	call program_kvectors

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

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

	; CTC 1 and 2 drive the SIO are are on a fixed 1.8Mhz clock. The
	; timer though is off the CPU clock so we just have to hope
	; everyone stays with 10MHz
	;
	; This is btw a lousy choice because with rhe /256 divider we get
	; 39062 clocks and 19531 is prime...
	;

	ld a,#0xB5
	out (CTC_CH3),a
	ld a,#217			; 180Hz ish (we'll drift slghly... )
	; TOOD add a subtle fudge factor to the clock to fix the drift
	out (CTC_CH3),a

	im 1				; set Z80 CPU interrupt mode 1 for now
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

; No way to page the ROM back in
_plt_monitor:
_plt_reboot:
	di
	halt

_int_disabled:
	.db	1
pagereg:
	.db	0
pagesave:
	.db	0

_gpio:
	.db	0x05			; CS high clock low MOSI high

plt_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop	de			; temporarily store return address
	pop	hl			; function argument -- base page number
	push	hl			; put stack back as it was
	push	de

	call	map_process

	; write zeroes across all vectors
	ld	hl,#0
	ld	de,#1
	ld	bc,#0x007f		; program first 0x80 bytes only
	ld	(hl),#0x00
	ldir

program_kvectors:
	; This is ok as for the bank 3 it's already JP

	ld	a,#0xC3			; JP instruction
	ld	(0x0000),a		; Must be present for NULL checker

	; now install the interrupt vector at 0x0038
	ld (0x0038),a
	ld hl,#interrupt_handler
	ld (0x0039),hl

	ld (0x0030),a			; rst 30h is old func call
	ld hl,#unix_syscall_entry
	ld (0x0031),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; Memory management
;=========================================================================

map_process:
map_process_di:
	ld	a,h
	or	l			; HL == 0?
	jr	z,map_kernel		; HL == 0 - map the kernel
	ld	a,(hl)
map_for_swap:
map_process_a:
	push	bc
	ld	(pagereg),a
	ld	c,a
	ld	a,(_gpio)
	and	#0x0F
	or	c
	ld	(_gpio),a
	out	(0x10),a
	pop	bc
	ret

map_process_always:
map_process_always_di:
	push	af
	ld	a,(_udata + U_DATA__U_PAGE)
	call	map_process_a
	pop	af
	ret

map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
map_kernel_a:
	xor	a
	call	map_process_a
	pop	af
	ret

map_restore:
	push	af
	ld	a,(pagesave)
	call	map_process_a
	pop	af
	ret

map_save_kernel:
	push	af
	ld	a,(pagereg)
	ld	(pagesave),a
	jr	map_kernel_a

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

outchar:
	push af
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
	ret
