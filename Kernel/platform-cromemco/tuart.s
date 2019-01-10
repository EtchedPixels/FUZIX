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
	push hl
	push bc
	ld bc,#0x4001
	in a,(c)
	out (c),b
	ld b,a
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
	; Do we need to mask the int off
	; if so need to cache 03 state (intmask) and add 0x20 (TBE)
	out (c),b
	pop bc
	pop hl
	pop af
	ei
	RET
;
;	Interrupt vector handler for port A receive ready
;
_tuart'X'_rx_ring:
	push af
	push hl
	push bc
	ld bc,#0x4001
	in a,(c)
	out (c),b
	ld b,a
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
	in a,(P)		; RR 0
	bit 6,a			; RDA is high if there is more to read
	jr c, tuart'X'_rx_next
	out (c),b
	pop bc
	pop hl
	pop af
	ei
	RET
tuart'X'_rx_over:
	ld a,(tuart'X'_error)
	or #0x04		; Fake an RX overflow bit
	ld (tuart'X'_error),a
	out (c),b
	pop bc
	pop hl
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
	rla			; RX space ?
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
	scf
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
