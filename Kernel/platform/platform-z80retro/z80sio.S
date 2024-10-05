;
;	Combined with the templates to make the serial driver.
;
	.export _sio_state
	.export _sio_dropdcd
	.export _sio_rxl
	.export _sio_txl

	.export siob_txd
	.export siob_status
	.export siob_rx_ring
	.export siob_special
	.export sioa_txd
	.export sioa_status
	.export sioa_rx_ring
	.export sioa_special

	.export sio_sp

	.data

; These are laid out and exposed as arrays to C
_sio_wr5:
_sioa_wr5:
	.byte 0xEA		; DTR, 8bit, tx enabled
_siob_wr5:
	.byte 0xEA		; DTR, 8bit, tx enabled
_sio_flow:
_sioa_flow:
	.byte 0			; Flow starts off
_siob_flow:
	.byte 0			; Flow starts off
_sio_state:
_sioa_state:
	.byte 0			; Last status report
_siob_state:
	.byte 0			; Last status report
_sio_dropdcd:
_sioa_dropdcd:
	.byte 0			; DCD dropped since last checked
_siob_dropdcd:
	.byte 0			; DCD dropped since last checked
_sio_rxl:
_sioa_rxl:
	.byte 0
_siob_rxl:
	.byte 0
_sio_txl:
_sioa_txl:
	.byte 0
_siob_txl:
	.byte 0
sio_sp:
	.word 0
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
;
;	Template values
;
;	1	Name for this SIO (eg 'A' or 'B' etc)
;	2	section/adr etc for the buffers (must be aligned correctly)
;		eg  ".abs\n.org 0x200\n"
;	3	ditto for the work registers, usually .common
;	4	Control port
;	5	Data port
;	6	Ret instruction for interrupt (ret or reti)
;	7	code to execute when entering handler (eg to bank the
;		buffers in). May well be nothing for common memory setup
;	8	code to unbank
;

	.abs
.org 0x200		; set section and addr for these
sioa_rx:
	.ds	128
sioa_tx:
	.ds	128

	.data		; set section for these

sioa_error:
	.byte	0
sioa_rxover:
	.byte	0
sioa_stat:
	.byte	0
sioa_txp:
	.word	sioa_tx
sioa_txe:
	.word	sioa_tx
sioa_rxp:
	.word	sioa_rx
sioa_rxe:
	.word	sioa_rx


;
;	C interface methods
;
	.export _sioa_txqueue
	.export _sioa_flow_off
	.export _sioa_flow_on
	.export _sioa_rx_get
	.export _sioa_error_get

;
;	Interrupt vector handler for port A transmit empty
;
sioa_txd:
	push af
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	ld a,(_sioa_txl)
	or a
	jr z, tx_a_none
	push hl
	dec a
	ld (_sioa_txl),a
	ld hl,(sioa_txp)
	ld a,(hl)
	out (SIOA_D),a
	inc l
	set 7,l
	ld (sioa_txp),hl
	pop hl
	jr txon_a
tx_a_none:
	ld a,0x28
	out (SIOA_C),a		; silence tx interrupt
txon_a:
	call map_restore
	ld sp,(sio_sp)
	pop af
	ei
	reti
;
;	Interrupt vector handler for port A receive ready
;
sioa_rx_ring:
	push af
	push hl
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
sioa_rx_next:
	in a,(SIOA_D)		; read ASAP
	ld l,a
	ld a,(_sioa_rxl)
	inc a
	jp m, a_rx_over
	ld (_sioa_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
	cp 0x60		; flow control threshold
	call z, _sioa_flow_on
	ld a,l
	ld hl,(sioa_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (sioa_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOA_C)		; RR 0
	rra
	jr c, sioa_rx_next
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti
a_rx_over:
	ld a,(sioa_error)
	or 0x20		; Fake an RX overflow bit
	ld (sioa_rxover),a
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti
;
;	Interrupt vector for a port A status change
;
sioa_status:
	; CTS or DCD change
	push af
	push hl
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	; RR0
	in a,(SIOA_C)
	ld (_sioa_state),a
	and 8
	jr z, no_dcd_drop_a
	; \DCD went high
	ld (_sioa_dropdcd),a		; Set the dcdflag
no_dcd_drop_a:
	; Clear the latched values
	ld a,0x10
	out (SIOA_C),a
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti

;
;	Interrupt vector for a port A error
;
sioa_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	ld a,1
	out (SIOA_C),a		; RR1 please
	in a,(SIOA_C)		; clear events
	ld (sioa_error),a	; Save error bits
	; Clear the latched values
	ld a,0xC0
	out (SIOA_C),a
	call map_restore
	ld sp,(sio_sp)
	pop af
	ei
	reti

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_sioa_txqueue:
	ld a,(_sioa_txl)
	or a
	jr z, sioa_direct_maybe	; if can tx now then do
	inc a
	jp m, txa_overflow
sioa_queue:
	ld (_sioa_txl),a
	ld a,l
	ld hl,(sioa_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (sioa_txe),hl
	ld l,0
	ret
txa_overflow:
	; some kind of flag for error
	ld l,1
	ret
sioa_direct_maybe:
	; check RR
	in a,(SIOA_C)
	and 0x04		; TX space ?
	; if space
	ld a,1
	jr z, sioa_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOA_D),a
	ld l,0
	ret
	; Call with DI

_sioa_flow_off:
	ld a,5
	out(SIOA_C),a		; WR 5
	ld a,(_sioa_wr5)
	out (SIOA_C),a		; Turn off RTS
	ret

_sioa_flow_on:
	ld a,5
	out(SIOA_C),a		; WR 5
	ld a,(_sioa_wr5)
	and 0xFD
	out (SIOA_C),a		; Turn off RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_sioa_rx_get:
	ld a,(_sioa_rxl)
	or a
	ret z
	dec a
	ld (_sioa_rxl),a
	ld hl,(sioa_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (sioa_rxe),hl
	ld l,a
	ret

	; DI required
_sioa_error_get:
	ld hl,sioa_error
	ld a,(hl)
	ld (hl),0
	ld l,a
	ret
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
;
;	Template values
;
;	1	Name for this SIO (eg 'A' or 'B' etc)
;	2	section/adr etc for the buffers (must be aligned correctly)
;		eg  ".abs\n.org 0x200\n"
;	3	ditto for the work registers, usually .common
;	4	Control port
;	5	Data port
;	6	Ret instruction for interrupt (ret or reti)
;	7	code to execute when entering handler (eg to bank the
;		buffers in). May well be nothing for common memory setup
;	8	code to unbank
;

	.abs
.org 0x300		; set section and addr for these
siob_rx:
	.ds	128
siob_tx:
	.ds	128

	.data		; set section for these

siob_error:
	.byte	0
siob_rxover:
	.byte	0
siob_stat:
	.byte	0
siob_txp:
	.word	siob_tx
siob_txe:
	.word	siob_tx
siob_rxp:
	.word	siob_rx
siob_rxe:
	.word	siob_rx


;
;	C interface methods
;
	.export _siob_txqueue
	.export _siob_flow_off
	.export _siob_flow_on
	.export _siob_rx_get
	.export _siob_error_get

;
;	Interrupt vector handler for port A transmit empty
;
siob_txd:
	push af
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	ld a,(_siob_txl)
	or a
	jr z, tx_b_none
	push hl
	dec a
	ld (_siob_txl),a
	ld hl,(siob_txp)
	ld a,(hl)
	out (SIOB_D),a
	inc l
	set 7,l
	ld (siob_txp),hl
	pop hl
	jr txon_b
tx_b_none:
	ld a,0x28
	out (SIOB_C),a		; silence tx interrupt
txon_b:
	call map_restore
	ld sp,(sio_sp)
	pop af
	ei
	reti
;
;	Interrupt vector handler for port A receive ready
;
siob_rx_ring:
	push af
	push hl
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
siob_rx_next:
	in a,(SIOB_D)		; read ASAP
	ld l,a
	ld a,(_siob_rxl)
	inc a
	jp m, b_rx_over
	ld (_siob_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
	cp 0x60		; flow control threshold
	call z, _siob_flow_on
	ld a,l
	ld hl,(siob_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (siob_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOB_C)		; RR 0
	rra
	jr c, siob_rx_next
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti
b_rx_over:
	ld a,(siob_error)
	or 0x20		; Fake an RX overflow bit
	ld (siob_rxover),a
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti
;
;	Interrupt vector for a port A status change
;
siob_status:
	; CTS or DCD change
	push af
	push hl
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	; RR0
	in a,(SIOB_C)
	ld (_siob_state),a
	and 8
	jr z, no_dcd_drop_b
	; \DCD went high
	ld (_siob_dropdcd),a		; Set the dcdflag
no_dcd_drop_b:
	; Clear the latched values
	ld a,0x10
	out (SIOB_C),a
	call map_restore
	ld sp,(sio_sp)
	pop hl
	pop af
	ei
	reti

;
;	Interrupt vector for a port A error
;
siob_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld (sio_sp),sp
	ld sp,istack_top
	call map_save_kernel
	ld a,1
	out (SIOB_C),a		; RR1 please
	in a,(SIOB_C)		; clear events
	ld (siob_error),a	; Save error bits
	; Clear the latched values
	ld a,0xC0
	out (SIOB_C),a
	call map_restore
	ld sp,(sio_sp)
	pop af
	ei
	reti

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_siob_txqueue:
	ld a,(_siob_txl)
	or a
	jr z, siob_direct_maybe	; if can tx now then do
	inc a
	jp m, txb_overflow
siob_queue:
	ld (_siob_txl),a
	ld a,l
	ld hl,(siob_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (siob_txe),hl
	ld l,0
	ret
txb_overflow:
	; some kind of flag for error
	ld l,1
	ret
siob_direct_maybe:
	; check RR
	in a,(SIOB_C)
	and 0x04		; TX space ?
	; if space
	ld a,1
	jr z, siob_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOB_D),a
	ld l,0
	ret
	; Call with DI

_siob_flow_off:
	ld a,5
	out(SIOB_C),a		; WR 5
	ld a,(_siob_wr5)
	out (SIOB_C),a		; Turn off RTS
	ret

_siob_flow_on:
	ld a,5
	out(SIOB_C),a		; WR 5
	ld a,(_siob_wr5)
	and 0xFD
	out (SIOB_C),a		; Turn off RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_siob_rx_get:
	ld a,(_siob_rxl)
	or a
	ret z
	dec a
	ld (_siob_rxl),a
	ld hl,(siob_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (siob_rxe),hl
	ld l,a
	ret

	; DI required
_siob_error_get:
	ld hl,siob_error
	ld a,(hl)
	ld (hl),0
	ld l,a
	ret
