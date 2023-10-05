;
;	The bit bang support routines for the Z180 Mini-ITX
;	(Keyboard)
;

	.globl _ps2kbd_get
	.globl _ps2kbd_put
	.globl _ps2kbd_beep

	.globl _kbsave

	.globl abort_sp

	.module ps2bitbang-rc2014-kbd

	.include "../../dev/ps2bitbang-z80.def"

	.area _DATA

abort_sp:
	.word	0
_kbsave:
	.byte	0

	.area _CODE

_ps2kbd_beep:
	ret

;
;	Keyboard: clock on bit 6 data on bit 7
;	Keyboard: data in on bit 7 of port C clock in on bit 6 of port C
;
.macro HOLD_OFF
	; Critical path is reading port C so we point C at that by default
	; We need port A
	dec	c
	dec	c
	in	a,(c)
	and	a,#0xBF		; force clock down
	or	a,#0x80		; float data
	out	(c),a
	inc	c
	inc	c
.endm

.macro GET_CLOCK
	in	a,(c)		; needs to be very fast
	and	#0x40		; caller already deals with timeout
.endm

.macro FLOAT_CLOCK
	dec	c
	dec	c
	in	a,(c)
	or	a,#0x40		; float the clock
	out	(c),a
	inc	c
	inc	c
.endm

.macro LOWER_CLOCK
	dec	c
	dec	c
	in	a,(c)
	and	a,#0xBF
	out	(c),a
	inc	c
	inc	c
.endm

.macro LOWER_DATA
	dec	c
	dec	c
	in	a,(c)
	and	a,#0x7F
	out	(c),a
	inc	c
	inc	c
.endm

ps2_handlers kbd 0x42 0x42 255 255 kbd_bit_in kbd_bit_out

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
	jp z, ps2kbd_timeout
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
;

kbd_bit_in:
	; Step 1: Wait for clock to go low
	call	read_port
	bit	6,a
	jr nz,  kbd_bit_in
	rla			; data bit into carry
	rr	e		; into the result
	push	af		; save the carry bit for the caller
kbd_bit_in_2:
	call	read_port
	bit	6,a
	jr z,	kbd_bit_in_2
	pop	af		; recover carry
	ret
;
;	The transmit protocol is basically the same except that we are the
;	one who puts a bit on the bus during low clock and the device
;	samples it on the high clock
;
kbd_bit_out:
	call	read_port
	bit 	6,a
	jr 	nz, kbd_bit_out
	dec	c
	dec	c
	in	a,(c)
	and	#0x3F
	or	#0x40			; we need to leave the clock floating
	rr	l
	jr	nc, kbdout0		; write a 0 bit then
	or	#0x80			; write a 1 bit
kbdout0:
	out	(c),a
	inc	c
	inc	c
	; Now wait until we enter clock high
kbd_bit_out_2:
	call	read_port
	bit	6,a
	jr	z, kbd_bit_out_2
	ret
