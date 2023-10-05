;
;	N8 Keyboard Inteface - another bitbanger's heaven
;
;	82C55 @0x84
;	Port C needs to be A out B in C high out C low out
;
;	Port B bit 2 data in
;	Port B bit 3 clock in
;
;	Port C bit 6 kbd data out
;	Port C bit 7 kbd clock out
;
;	The bit bang support routines for the N8 platform
;	(Mouse)
;

	.globl _ps2mouse_get
	.globl _ps2mouse_put

	.globl abort_sp

	.module ps2bitbang-n8-mouse

	.include "../../dev/ps2bitbang-z80.def"

	.area _CODE

;	Mouse: clock on bit 7 data in bit 6, low is pulled down
;	inpt bits are on port 0x85 data 2 clock 3
;
.macro HOLD_OFF
	in	a,(0x86)
	and	a,#0x7F		; force clock down
	or	a,#0x40		; float data
	out	(0x86),a
.endm

.macro GET_CLOCK
	in	a,(0x85)	; needs to be very fast
	and	#8		; caller already deals with timeout
.endm

.macro FLOAT_CLOCK
	in	a,(0x86)
	or	a,#0x80		; float the clock
	out	(0x86),a
.endm

.macro LOWER_CLOCK
	in	a,(0x86)
	and	a,#0x7F		; pull the clock down
	out	(0x86),a
.endm

.macro LOWER_DATA
	in	a,(0x86)
	and	a,#0xBF		; pull the data line down
	out	(0x86),a
.endm

ps2_handlers mouse 0x00 0x00 255 255 mouse_bit_in mouse_bit_out

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
	jp z, ps2mouse_timeout
	exx
	in a,(0x85)
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
;

mouse_bit_in:
	; Step 1: Wait for clock to go low
	call	read_port
	bit	3,a	
	jr nz,  mouse_bit_in
	and	#4
	add	d		; force carry if set
	rr	e		; into the result
	push	af		; save the carry bit for the caller
mouse_bit_in_2:
	call	read_port
	bit	3,a
	jr z,	mouse_bit_in_2
	pop	af		; recover carry
	ret
;
;	The transmit protocol is basically the same except that we are the
;	one who puts a bit on the bus during low clock and the device
;	samples it on the high clock
;
mouse_bit_out:
	call	read_port
	bit 	3,a
	jr 	nz, mouse_bit_out
	in	a,(0x86)		; get the state of the other bits
	and	#0xCF
	or	#0x20			; we need to leave the clock floating
	rr	l
	jr	nc, mouseout0		; write a 0 bit then
	or	#4			; write a 1 bit
mouseout0:
	out	(0x86),a
	; Now wait until we enter clock high
mouse_bit_out_2:
	call	read_port
	bit	3,a
	jr	z, mouse_bit_out_2
	ret
