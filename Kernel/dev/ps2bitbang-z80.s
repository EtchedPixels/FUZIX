;
;	PS/2 protocol bit bang framework
;

	.include "ps2bitbang-z80.def"

	.globl ps2_read_port

	.globl ps2_receive_code
	.globl ps2_send_code

	.area _CODE

;
;	Must preseve registers
;
set_timeout:
	exx
	ld hl,#0xFFFF		; hard coded for now
	exx
	ret

timeout:
	exx
	ld hl, #0xFFFD
	ld sp, (abort_sp)
	; Jam the clock
	ld l, HOLD_OFF(ix)
	ld h, HOLD_OFF+1(ix)
	call jphl
	; All our callers start by pushing IX so we can recover it safely
	pop ix
	pop iy
	ret

;
;	Helpers for calling functions
;
jpiy:
	.byte 0xFD
jphl:
	jp (hl)

;
;	This lets us avoid worrying about timeouts everywhere, instead
;	we throw an exception. Basically in a,(c) with a timeout.
;
;	C is the port, returns in A. Other registers preserved
;
ps2_read_port:
	exx
	dec hl
	ld a,h
	or l
	jp z, timeout
	exx
	in a,(c)
	ret

;
;	The higher level protocol
;
;	Caller passes table in HL
;
ps2_receive_code:
	push iy
	push ix
	push hl
	pop ix

	; This entry point is used when we are doing a send/receive pair
receive_after_tx:
	; Get the bit receiving function into IY

	ld l, RECEIVE_BIT(ix)
	ld h, RECEIVE_BIT+1(ix)
	push hl
	pop iy

	ld c, PORT(ix)
	ld (abort_sp),sp

	; Float the clock
	ld l, FLOAT_CLOCK(ix)
	ld h, FLOAT_CLOCK+1(ix)
	call jphl

	ld l, GET_CLOCK(ix)
	ld h, GET_CLOCK+1(ix)

	ld b, RESPONSE_DELAY(ix)
	; Allow the keyboard time to respond
receive_wait:
	call jphl
	jr z, data_receive
	djnz receive_wait

	; Keyboard is not talking to us
	; Stop it from talking until the next poll
	ld de, #-1		; error code for 'nowt received'

receive_done:
	ld l, HOLD_OFF(ix)
	ld h, HOLD_OFF+1(ix)
	call jphl

	ex de,hl		; error into HL
	pop ix
	pop iy
	ret

	; The keyboard lowered the clock - that means it will be sending
	; us a code. Each code it sends consists of
	;
	; start bit 0
	; 8 data bits low-high order
	; parity (odd)
	; stop bit (1)

data_receive:
	ld d,#0xFF		; trick for carry conversion with some
				; bit functions
	call set_timeout

	ld b,#9			; start and 8 data bits
read_bits:
	call jpiy		; call the bit in function
	djnz read_bits
	; Carry is now the start bit (must be 0)
	jr c, bad_start_bit
	; Check what parity should be
	ld a,e
	or a
	ld h,#0x80		; expect a 1 bit for even
	jp pe, even_parity
	ld h,#0x00
	ld l,e			; save character
even_parity:
	call jpiy		; read the parity bit
	ld a,e
	and #0x80
	cp h			; check as expected
	jr z, parity_good
	ld de,#-2		; parity error
	jr receive_done

parity_good:
	call jpiy		; we must read the stop bit or the keyboard
				; will think we failed the receive
	ld h,#0
	ex de,hl		; save char in DE
	jr receive_done

bad_start_bit:
	ld de,#-3
	jr receive_done

;
;	The transmit protocol for sending data to the keyboard
;	Passed a control table in HL and a byte in A
;
ps2_send_code:
	push iy
	push ix
	push hl
	pop ix

	push af
	ld a,#0x10
	out (0xFD),a
	pop af

	ld c, PORT(IX)
	; Get the bit sending function into IY

	ld l, SEND_BIT(ix)
	ld h, SEND_BIT+1(ix)
	push hl
	pop iy

	ld d, a
	call set_timeout
	;
	; Pull the clock low to get attention
	;
	ld l, LOWER_CLOCK(ix)
	ld h, LOWER_CLOCK+1(ix)
	call jphl
	;
	;	Wait 100us with the clock pulled low for the other end to
	;	notice the fact we are banging on the door

	ld b, POLL_DELAY(ix)
send_wait_1:
	djnz send_wait_1
	;
	;	Pull data low as well
	;
	ld l, LOWER_DATA(ix)
	ld h, LOWER_DATA+1(ix)
	call jphl
	;
	;	Release the clock
	;
	ld l, FLOAT_CLOCK(ix)
	ld h, FLOAT_CLOCK+1(ix)
	call jphl
	;
	;	The host will now start sending us clocks and we send bits
	;
	;	The data pull down will act as the start bit
	;
	ld b,#8		;	Send 8 data bits
	ld l, d		;	save byte we are sending

send_byte:
	call jpiy	;	send a bit
	djnz send_byte
	ld a, d
	or a		;	Which parity ?
	ld l, #1
	jp pe, out_parity_1
	dec l
out_parity_1:
	call jpiy	;	Send the parity bit
	ld l, #1
	call jpiy	;	Send the stop bit

	;
	;	We should now receive a handshake from the other
	;	end. We know the data line is floating as we sent a 1
	;

	ld l, RECEIVE_BIT(ix)
	ld h, RECEIVE_BIT+1(ix)
	call jphl

	 rr e		;	check received bit

	jp nc, receive_after_tx

	;	The keyboard did not ack our request. So we are dome
	ld de, #-4
	jp receive_done

	;
	;	The keyboard received our byte and confirmed receipt
	;

	.area _DATA
abort_sp:
	.word	0