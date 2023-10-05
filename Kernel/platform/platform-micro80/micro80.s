
        .module micro80

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
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
	.globl ___sdcc_enter_ix

	; exported debugging tools
	.globl inchar
	.globl outchar

        .include "kernel.def"
        .include "../kernel-z80.def"


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
	ld hl,#128
	ld (_ramsize), hl
	ld hl,#60
	ld (_procmem), hl

	ld a,#3
	out (0xEE),a
	in a,(0xEF)
	and #0xFC			; make sure CS lines are high
	out (0xEF),a
	ld a,#2
	out (0xEE),a
	ld a,#0xF0			; CS1 is low for 1000-FFFF
	out (0xEF),a

	ld a,#3				; set the register to MCR
	out (0xEE),a			; for speed

	;
	;	We run the kernel with CS1 high and user with CS1 low
	;	and the low 4K common
	;

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0C00 + SIOB_C		; and to SIOB_C
	otir

	ld hl,#siob_txd
	ld (0xff0),hl			; SIO B TX empty
	ld hl,#siob_status
	ld (0xff2),hl			; SIO B External status
	ld hl,#siob_rx_ring
	ld (0xff4),hl			; SIO B Receive
	ld hl,#siob_special
	ld (0xff6),hl			; SIO B Special
	ld hl,#sioa_txd
	ld (0xff8),hl			; SIO A TX empty
	ld hl,#sioa_status
	ld (0xffa),hl			; SIO A External status
	ld hl,#sioa_rx_ring
	ld (0xffc),hl			; SIO A Received
	ld hl,#sioa_special
	ld (0xffe),hl			; SIO A Special

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

	ld (0x0000),a
	ld hl,#null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	ld hl,#rstblock			; Install rst helpers
	ld de,#8
	ld bc,#32
	ldir
	;
	; Defense in depth - shut everything up first
	;

	ld a,#0x43
	out (CTC_CH0),a			; set CH0 mode
	out (CTC_CH1),a			; set CH1 mode
	out (CTC_CH2),a			; set CH2 mode
	out (CTC_CH3),a			; set CH3 mode

	;
	; Set up timer for 100Hz
	; The CTC is clocked at 3.6864MHz
	;
	; IM2 vector for the CTC
	ld hl, #spurious
	ld (0xfe0),hl			; CTC vectors
	ld (0xfe2),hl
	ld (0xfe6),hl
	ld hl, #interrupt_handler	; Standard tick handler
	ld (0xfe4),hl

	ld a,#0xE0
	out (CTC_CH0),a	; set the CTC vector
	ld a,#0xB5
	out (CTC_CH2),a
	ld a,#144
	out (CTC_CH2),a	; 100 Hz

	;
	; Set up counter CH3 to count CH2 - assumes ZC2 is jumpered to TRG3
	;
	ld a,#0x47
	out (CTC_CH3),a
	ld a,#255
	out (CTC_CH3),a

        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	;
	;	PIO
	;
	;	A 7-3:	Joystick
	;	A 2-0:	SPI (CLK, MOSI, MISO)
	;	B 7-3:  Joystick
	;	B 2-0:	SPI select
	;

	ld a,#0xFF
	out (PIOA_C),a		; bitwise
	ld a,#0xF9		; CLK and MOSI are outputs
	out (PIOA_C),a
	ld a,#0x07
	out (PIOA_C),a		; no interrupts
	ld a,#0xFF
	out (PIOB_C),a		; bitwise
	ld a,#0xF8		; SPI selects are outputs
	out (PIOB_C),a
	ld a,#0x07
	out (PIOB_C),a

	ld a,#0x0f
	ld i,a
	im 2
	ret

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x1F
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte RTS_LOW
	.byte 0x02
	.byte 0xF0		; IRQ vector written to port B

;=========================================================================
; Kernel code
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	; FIXME: we can flip the CS lines around and also generate a reset
	; on the bus too.
	di
	halt

_int_disabled:
	.db 1

plt_interrupt_all:
	ret

; install interrupt vectors - no-op as in common low space
_program_vectors:
	ret

spurious:
	ei
	reti

;=========================================================================
; Memory management
;=========================================================================

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
map_proc_always_di:
	push af
	in a,(0xEF)
	or #0x02		; CS1 enable for user space
	out (0xEF),a
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
map_kernel_di:
map_kernel_restore:
	push af
	in a,(0xEF)
	and #0xFD		; CS1 disable for kernel
	out (0xEF),a
	pop af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push af
	ld a,(map_save)
	out (0xEF),a
	pop af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push af
	in a,(0xEF)
	ld (map_save),a
	and #0xFD
	out (0xEF),a
	pop af
	ret


map_save:
	.db 0
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


;=========================================================================
; inchar - Wait for character on UART, return in A
; Inputs: none
; Outputs: A - received character, F destroyed
;=========================================================================
inchar:
        xor a                           ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar			; no data, wait
	in a,(SIOA_D)   		; read the character from the UART
	ret

;=========================================================================
;
;	Logic for IM2 interrupt driven SIO
;
;=========================================================================

	.area _SERIALDATA

	.globl _sio_state
	.globl _sio_dropdcd
	.globl _sio_rxl
	.globl _sio_txl

; These are laid out and exposed as arrays to C
_sio_wr5:
_sioa_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_siob_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_sio_flow:
_sioa_flow:
	.db 0			; Flow starts off
_siob_flow:
	.db 0			; Flow starts off
_sio_state:
_sioa_state:
	.db 0			; Last status report
_siob_state:
	.db 0			; Last status report
_sio_dropdcd:
_sioa_dropdcd:
	.db 0			; DCD dropped since last checked
_siob_dropdcd:
	.db 0			; DCD dropped since last checked
_sio_rxl:
_sioa_rxl:
	.db 0
_siob_rxl:
	.db 0
_sio_txl:
_sioa_txl:
	.db 0
_siob_txl:
	.db 0

	.include "../dev/z80sio.s"

sio_ports a
sio_ports b

	.area _COMMONMEM

;
;	We don't do any clever IRQ re-entry tricks so we can use the istack.
;	If we ever do soft interrupts and take the sio during a timer int
;	we will need a small (16 byte or so) private sio work stack and to
;	be very careful with our stack handling on switches.
;

.macro switch
.endm

.macro switchback
.endm

sio_handler_im2	a, SIOA_C, SIOA_D, reti
sio_handler_im2 b, SIOB_C, SIOB_D, reti

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
