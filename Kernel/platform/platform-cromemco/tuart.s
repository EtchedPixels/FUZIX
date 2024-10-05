;
;	Based on the SIO driver but for the Cromemco TU-ART
;
;	The big difference is that we have to do a minimal task switch
;	in order to access our data as we don't have a true common and also
;	common space is precious
;
;	For speed we flip bank but not stack so once we flip we must not
;	use the stack until we flip back.
;
;	Note there a couple of places we do things a particular way to work
;	around bugs in the emultion of the TU-ART in Z80Pack
;

.macro tuart_ports X

	.globl _tuart'X'_rxl
	.globl _tuart'X'_txl

;
;	Must be aligned. Need not be common
;

	.area	_SERIAL
tuart'X'_rx:
	.ds	64
tuart'X'_tx:
	.ds	64

;
;	We flip maps so we don't need this in common
;
	.area _INTDATA
tuart'X'_error:
	.db	0
tuart'X'_rxover:
	.db	0
tuart'X'_txp:
	.dw	tuart'X'_tx
tuart'X'_txe:
	.dw	tuart'X'_tx
tuart'X'_rxp:
	.dw	tuart'X'_rx
tuart'X'_rxe:
	.dw	tuart'X'_rx
_tuart'X'_rxl:
	.db	0
_tuart'X'_txl:
	.db	0
_tuart'X'_error:
	.db	0

.endm

.macro tuart_handler_im2 X P lh RET

;
;	C interface methods
;
	.globl _tuart'X'_txqueue
	.globl _tuart'X'_rx_get
	.globl _tuart'X'_error_get
	.globl _tuart'X'_txd
	.globl _tuart'X'_rx_ring

	.area _COMMONMEM
;
;	Interrupt vector handler for port A transmit empty
;
_tuart'X'_txd:
	push af
	push bc
	push hl
	; We have a polled console so the console could in fact beat us
	; to the buffer. If so we just return - the console byte transmit
	; completion will call us again.
	in a,(P)
	rla
	jr nc, console'X'_race
	in a,(0x40)
	ld c,a
	ld a,#0x1
	out (0x40),a
	ld a,(_tuart'X'_txl)
	or a
	jr z, tx_'X'_none
	dec a
	ld (_tuart'X'_txl),a
	ld hl,(tuart'X'_txp)
	ld a,(hl)
	out (P + 1),a
	inc l
	set 6,l
	lh 7,l			; Force to upper or lower half
	ld (tuart'X'_txp),hl
tx_'X'_none:
	ld a,c
	out (0x40),a
console'X'_race:
	pop hl
	pop bc
	pop af
	ei
	RET
;
;	Interrupt vector handler for port A receive ready
;
_tuart'X'_rx_ring:
	push af
	push bc
	push hl
	in a,(0x40)
	ld c,a
	ld a,#1
	out (0x40),a
tuart'X'_rx_next:
	in a,(P)		; status
	and #7
	jp z, tuart'X'_no_error
	ld hl,#_tuart'X'_error	; accumulate error bits
	or (hl)
	ld (hl),a
tuart'X'_no_error:
	in a,(P + 1)		; read ASAP
	ld l,a
	ld a,(_tuart'X'_rxl)
	inc a
	bit 6,a
	jp nz, tuart'X'_rx_over
	ld (_tuart'X'_rxl),a
	ld a,l
	ld hl,(tuart'X'_rxp)
	ld (hl),a
	inc l
	res 6,l
	lh 7,l
	ld (tuart'X'_rxp),hl
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	ld a,c
	out (0x40),a
	pop hl
	pop bc
	pop af
	ei
	RET
tuart'X'_rx_over:
	ld a,(tuart'X'_error)
	or #0x04		; Fake an RX overflow bit
	ld (tuart'X'_error),a
	ld a,c
	out (0x40),a
	pop hl
	pop bc
	pop af
	ei
	RET

	.area _CODE

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_tuart'X'_txqueue:
	ld a,(_tuart'X'_txl)
	or a
	jr z, tuart'X'_direct_maybe	; if can tx now then do
	inc a
	bit 6,a
	jp nz, tx'X'_overflow
tuart'X'_queue:
	ld (_tuart'X'_txl),a
	ld a,l
	ld hl,(tuart'X'_txe)
	ld (hl),a
	inc l
	set 6,l
	lh 7,l
	ld (tuart'X'_txe),hl
	ld l,#0
	ret
tx'X'_overflow:
	; some kind of flag for error
	ld l,#1
	ret
tuart'X'_direct_maybe:
	; check RR
	in a,(P)
	rla			; TX space ?
	; if space
	ld a,#1
	jr nc, tuart'X'_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (P + 1),a
	ld l,#0
	ret

	; DI required
	; Returns char in L
_tuart'X'_rx_get:
	ld a,(_tuart'X'_rxl)
	or a
	ret z
	dec a
	ld (_tuart'X'_rxl),a
	ld hl,(tuart'X'_rxe)
	ld a,(hl)
	inc l
	res 6,l
	lh 7,l
	ld (tuart'X'_rxe),hl
	ld l,a
	ret

	; DI required
_tuart'X'_error_get:
	ld hl,#tuart'X'_error
	ld a,(hl)
	ld (hl),#0
	ld l,a
	ret


.endm
