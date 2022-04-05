;
;	The bit bang support routines for the RC2014 platform
;	(mouse)
;

	.globl _ps2bmouse_get
	.globl _ps2bmouse_put

	.globl _kbsave
	.globl abort_sp

	.module ps2bitbang-rc2014-mouse

	.include "ps2bitbang-z80.def"

	.area _CODE


;
;	Mouse: clock on bit 2 data in bit 3, low is pulled down
;	clock input is on bit 0 data on 1
;
.macro HOLD_OFF
	ld	a,(_kbsave)
	and	a,#0xFB		; force clock down
	or	a,#0x08		; float data
	ld	(_kbsave),a
	out	(c),a
.endm

.macro GET_CLOCK
	in	a,(c)		; needs to be very fast
	and	#1		; caller already deals with timeout
.endm

.macro FLOAT_CLOCK
	ld	a,(_kbsave)
	or	a,#0x04		; float the clock
	ld	(_kbsave),a
	out	(c),a
.endm

.macro LOWER_CLOCK
	ld	a,(_kbsave)
	and	a,#0xFB		; pull the clock down
	ld	(_kbsave),a
	out	(c),a
.endm

.macro LOWER_DATA
	ld	a,(_kbsave)
	and	a,#0xF7		; pull the data line down
	ld	(_kbsave),a
	out	(c),a
.endm

ps2_handlers bmouse 0xBB 0xBB 255 255 mouse_bit_in mouse_bit_out

;
;	This lets us avoid worrying about timeouts everywhere, instead
;	we throw an exception. Basically in a,(c) with a timeout.
;
;	C is the port, returns in A. Other registers preserved
;
read_port:
	exx
	dec hl
	ld a,h
	or l
	jp z, ps2bmouse_timeout
	exx
	in a,(c)
	ret


;	bit_in
;	passed a port in C, shifts E left, adds the bit to the E register and
;	preserves the previous top bit in carry.
;
;	bit_out
;	sends the low bit of l shifts l right.
;
;	In both cases you need to wait for the clock to go low, act and wait
;	for the clock to return
;
;	On entry the clock and data lines will be floating and the clock
;	should be left floating.
;
;	Register usage
;	C is PORT
;	D is 0xFF (for converting bits to carry)
;	E is the data on receive
;	L is the data on send
;
;	B, E, H, L should be preserved or modified as stated
;	A is scratch. HL' is used by the timeout code.
;
;	-------------------------------------------------
;
;
;	Receive a bit. Wait for the clock to go low, collect the bit, and
;	then wait fro the clock to go high. We add the bit to high and take
;	care to preserve the carry from that shift
;
;	C is the port, IX holds the control structure
;	The bud 
;

mouse_bit_in:
	; Step 1: Wait for clock to go low
	call	read_port
	rra
	jr c,  mouse_bit_in
	rr	c		; data into carry
	rr	e		; into the result
	push	af		; save the carry bit for the caller
mouse_bit_in_2:
	call	read_port
	rra
	jr nc,	mouse_bit_in_2
	pop	af		; recover carry
	ret
;
;	The transmit protocol is basically the same except that we are the
;	one who puts a bit on the bus during low clock and the device
;	samples it on the high clock
;
mouse_bit_out:
	call	read_port
	rra
	jr 	c, mouse_bit_out
	ld	a, (_kbsave)		; get the state of the other bits
	and	#0xF7
	or	#4			; we need to leave the clock floating
	rr	l
	jr	nc, mouseout0		; write a 0 bit then
	or	#8			; write a 1 bit
mouseout0:
	out	(c),a
	ld	(_kbsave),a
	; Now wait until we enter clock high
mouse_bit_out_2:
	call	read_port
	rra
	jr	nc, mouse_bit_out_2
	ret
