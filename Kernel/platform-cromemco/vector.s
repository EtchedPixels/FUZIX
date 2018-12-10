;
;	Interrupt handlers. These will get replaced by some proper FIFO
; logic
;

	.area _COMMONMEM
	.module vector

	.globl _rx0a_char
	.globl _rx0a_int
	.globl _tx0a_int
	.globl interrupt_high

	.globl _uart0a_rx
	.globl _uart0a_txdone
	.globl _uart0a_timer4

_uart0a_rx:			; 0xE7
	push af
	push bc
	push de
	ld bc,#0x8140
	in d,(c)
	out (c),b
	in a,(1)
	ld (_rx0a_char),a
	ld a,#1
	ld (_rx0a_int),a
	out (c),d
	pop de
	pop bc
	pop af
	ei
	reti

_uart0a_txdone:			; 0xEF
	push af
	push bc
	push de
	ld bc,#0x8140
	in d,(c)
	out (c),b
	ld a,#1
	ld (_tx0a_int),a
	out (c),d
	pop de
	pop bc
	pop af
	ei
	reti

_uart0a_timer4:
	jp interrupt_high

_rx0a_char:
	.byte 0
_rx0a_int:
	.byte 0
_tx0a_int:
	.byte 0
