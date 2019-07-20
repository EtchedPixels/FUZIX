;
;	SIO2 IM2 console driver
;
;	The buffers are 256 bytes per channel and page aligned. The lower
;	half is used for receive the upper for transmit. Both are rings.

;
;	Transmit data from the queue. We need a stop system for this but
;	actually the logical one might be to just turn that IRQ off, so long
;	as we remember to kickstart it again properly. Check if it's enough
;	to just unmask the IRQ bit ?
;
;	All of this lives in common space so we don't bank switch so much.

.macro sio_ports X

	.area	_SERIAL
sio'X'_rx:
	.ds	128
sio'X'_tx:
	.ds	128

	.area _SERIALDATA
sio'X'_error:
	.db	0
sio'X'_rxover:
	.db	0
sio'X'_stat:
	.db	0
sio'X'_txp:
	.dw	sio'X'_tx
sio'X'_txe:
	.dw	sio'X'_tx
sio'X'_rxp:
	.dw	sio'X'_rx
sio'X'_rxe:
	.dw	sio'X'_rx

.endm

.macro sio_handler_im2 X CP DP RET

;
;	C interface methods
;
	.globl _sio'X'_txqueue
	.globl _sio'X'_flow_control_off
	.globl _sio'X'_flow_control_on
	.globl _sio'X'_rx_get
	.globl _sio'X'_error_get

;
;	Interrupt vector handler for port A transmit empty
;
sio'X'_txd:
	push af
	switch
	ld a,(_sio'X'_txl)
	or a
	jr z, tx_'X'_none
	push hl
	dec a
	ld (_sio'X'_txl),a
	ld hl,(sio'X'_txp)
	ld a,(hl)
	out (DP),a
	inc l
	set 7,l
	ld (sio'X'_txp),hl
	pop hl
tx_'X'_none:
	ld a,#0x28
	out (CP),a		; silence tx interrupt
	switchback
	pop af
	ei
	RET
;
;	Interrupt vector handler for port A receive ready
;
sio'X'_rx_ring:
	push af
	push hl
	switch
sio'X'_rx_next:
	in a,(DP)		; read ASAP
	ld l,a
	ld a,(_sio'X'_rxl)
	inc a
	jp m, 'X'_rx_over
	ld (_sio'X'_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
	cp #0x60		; flow control threshold
	call z, _sio'X'_flow_control_on
	ld a,l
	ld hl,(sio'X'_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (sio'X'_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(CP)		; RR 0
	rra
	jr c, sio'X'_rx_next
	switchback
	pop hl
	pop af
	ei
	RET
'X'_rx_over:
	ld a,(sio'X'_error)
	or #0x20		; Fake an RX overflow bit
	ld (sio'X'_rxover),a
	switchback
	pop hl
	pop af
	ei
	RET
;
;	Interrupt vector for a port A status change
;
sio'X'_status:
	; CTS or DCD change
	push af
	push hl
	switch
	; RR0
	in a,(CP)
	ld (_sio'X'_state),a
	and #8
	jr z, no_dcd_drop_'X
	; \DCD went high
	ld (_sio'X'_dropdcd),a		; Set the dcdflag
no_dcd_drop_'X':
	; Clear the latched values
	ld a,#0x10
	out (CP),a
	switchback
	pop hl
	pop af
	ei
	RET

;
;	Interrupt vector for a port A error
;
sio'X'_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	switch
	ld a,#1
	out (CP),a		; RR1 please
	in a,(CP)		; clear events
	ld (sio'X'_error),a	; Save error bits
	; Clear the latched values
	ld a,#0xC0
	out (CP),a
	switchback
	pop af
	ei
	RET

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_sio'X'_txqueue:
	ld a,(_sio'X'_txl)
	or a
	jr z, sio'X'_direct_maybe	; if can tx now then do
	inc a
	jp m, tx'X'_overflow
sio'X'_queue:
	ld (_sio'X'_txl),a
	ld a,l
	ld hl,(sio'X'_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (sio'X'_txe),hl
	ld l,#0
	ret
tx'X'_overflow:
	; some kind of flag for error
	ld l,#1
	ret
sio'X'_direct_maybe:
	; check RR
	in a,(CP)
	and #0x04		; TX space ?
	; if space
	ld a,#1
	jr z, sio'X'_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (DP),a
	ld l,#0
	ret
	; Call with DI

_sio'X'_flow_control_off:
	ld a,#5
	out(CP),a		; WR 5
	ld a,(_sio'X'_wr5)
	out (CP),a		; Turn off RTS
	ret

_sio'X'_flow_control_on:
	ld a,#5
	out(CP),a		; WR 5
	ld a,(_sio'X'_wr5)
	and #0xFD
	out (CP),a		; Turn off RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_sio'X'_rx_get:
	ld a,(_sio'X'_rxl)
	or a
	ret z
	dec a
	ld (_sio'X'_rxl),a
	ld hl,(sio'X'_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (sio'X'_rxe),hl
	ld l,a
	ret

	; DI required
_sio'X'_error_get:
	ld hl,#sio'X'_error
	ld a,(hl)
	ld (hl),#0
	ld l,a
	ret

.endm
