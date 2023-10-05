;
;	Define IOBASE to be the I/O base address configured for the 68HC11
;	(F000 on our RC2014 board)
;
#include "kernel.def"

;
;	Option bank switch helpers for the IRQ
;

		.mode mshort

		.section .common

	.globl sci_intr
	.globl sci_tx_queue
	.globl sci_tx_space
	.globl sci_tx_console
	.globl sci_rx_get
	.globl sci_error_get
;
;
;	RX queue lives at xx00-7F
;	TX queue lives at xx80-FF
;
;	This is designed to be used with the sci driver permanently mapped
;	and the sci data likewise. If not then the SCI_SWITCH/SCI_SWITCHBACK
;	defines allow insertion of bank switch code in the IRQ.
;
;	The SCI has its own interrupt but there isn't a way to mask \IRQ
;	internally so our ability to use this as a high priority interrupt
;	is very limited on platforms that use \IRQ.
;

sci_intr:
	tsx
	ldaa	0xF000
	ldab	#0x88
	stab	0xF000		; move to kernel bank
	lds	#sci_stack
	pshx			; save old stack on new
	psha			; save old bank on new
	ldx #IOBASE
	brclr 0x2E,x 0x20 norx
	ldaa 0x2F,x		; get the char ASAP
	inc sci_rxl
	bmi rxl_over		; 0x80 or higher - overrun
	; Could check here to do RTS/CTS
	ldy sci_rxp
	staa ,y
	inc sci_rxp+1
	bmi fixrxp		; rxp is now xx80 so fix
norx:	brclr 0x2E,x 0x0E chktx
	; error do we care ?
	ldaa 0x2E,x		; merge bits into error
seterr:
	oraa sci_err
	staa sci_err
chktx:
	brclr 0x2E,x 0x80 done	; TX not yet empty
	ldaa sci_txl		; anything to send ?
	beq txdone
	dec sci_txl
	ldy sci_txp		; grab a byte
	ldaa ,y
	staa 0x2F,x		; send it
	inc sci_txp+1		; move on
	bne done
	ldaa #0x80		; wrapped FF->0 so fix it back
	staa sci_txp+1
done:
	pula			; get old bank
	pulx			; and old stack
	staa	0xF000		; switch to old bank
	txs			; recover stack
	rti			; home time
txdone:
	bclr 0x2D,x 0x80	; clear TIE as we have nothing to send
	bra done
fixrxp:
	clr sci_rxp+1		; wrap to 7bits
	bra norx
rxl_over:
	dec sci_rxl		; we bumped it before checking
	ldaa #0x88		; Overflow bit
	bra seterr


;
;	Queue a byte to the serial queue. Returns 0 if successful, 1 if that
;	would overrun the buffer. If we the device is idle we send the byte
;	immediately
;
;	This one we disable interrupts for because of the potential races
;	on the queue
;
sci_tx_queue:
	tpa
	psha
	sei
	ldaa sci_txl
	beq sci_direct
txdoq:
	inca
	bmi txover
	inc sci_txl
	ldx sci_txe
	stab ,x
	inc sci_txe+1
	bne sci_txe_ok
	ldab #0x80		; wrap to start
	stab sci_txe+1
sci_txe_ok:
	clrb
	pula
	tap
	rts
txover:
	pula
	tap
	ldab #1
	rts
sci_direct:
	ldx #IOBASE
	brclr 0x2E,x 0x80 txdoq
	bset 0x2D,x 0x80	; TIE enable
	stab 0x2F,x
	clrb
	pula
	tap
	rts
;
;	Is the tx queue full ?
;
sci_tx_space:
	ldab #0x7F
	subb sci_txl
	rts
;
;	Send a kernel console logging byte. Interrupts may be off at this
;	point so we have to work harder and block
;
sci_tx_console:
	ldx #IOBASE
	tpa
	; Wait for pending character
conwait:
	brclr 0x2E,x 0x80 conwait
	; Check with interrupts off to avoid a race
	sei
	brclr 0x2E,x 0x80 conwait2
	stab 0x2F,x
	; Our write will have cleared any left over transmit interrupt
	; driving the queue. Turn the tx irq back on - the irq will turn
	; itself off if not needed
	bset 0x2D,x 0x80
	tap
	rts
conwait2:
	tap
	bra conwait

;
;	Check if there is a byte in the queue we can use.
;
;	We rely upon the fact that whilst on some setups this routine could
;	be interrupted by the sci interrupt the reverse is not true. Thus
;	sci_rxl is always seen as valid, and we merely need to atomically
;	decrement it.
;
sci_rx_get:
	ldaa #0xFF
	ldab sci_rxl		; check rx queue is empty
	beq nochar
	dec sci_rxl		; so we can't be interrupted mid update
	ldx sci_rxe		; rxe is only manipulated by rx_get
	clra
	ldab ,x			; and rx_get is not re-entrant
	inc sci_rxe+1
	bpl rxe_ok
	clr sci_rxe+1
nochar:
rxe_ok:
	rts

;
;	Retrieve and clear error bits
;
sci_error_get:
	tpa			; There is unfortunately no bclr ea reg
	sei
	ldab sci_err
	clr sci_err
	tap
	andb #0x0E		; Error bits
	rts
