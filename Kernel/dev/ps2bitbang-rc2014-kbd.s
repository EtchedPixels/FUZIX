;
;	The bit bang support routines for the RC2014 platform
;	(Keyboard)
;

	.globl _ps2kbd_get
	.globl _ps2kbd_put
	.globl _ps2kbd_beep

	.globl _kbsave

	.globl abort_sp

	.module ps2bitbang-rc2014-kbd

	.include "ps2bitbang-z80.def"

	.area _DATA

abort_sp:
	.word	0
_kbsave:
	.byte	0

	.area _CODE

_ps2kbd_beep:

	ld	a, #0xBB
	ld	c,a		; port
	ld	b,#0		; frequency
	ld	de,#150		; loops

beeper:
	ld a,	#0xF0		; vol 15
	out	(c),a

wait1:
	ex	(sp),hl		; These happen an even number of times
	djnz	wait1

	xor	a		; vol 0
	out	(c),a

wait0:
	ex (sp),hl
	djnz wait0

	dec de			; tone length
	ld a,d
	or e
	jr nz, beeper

	ret

;
;	Keyboard: clock on bit 0 data in bit 1, low is pulled down
;	clock input is on bit 2
;
.macro HOLD_OFF
	ld	a,(_kbsave)
	and	a,#0xFE		; force clock down
	or	a,#0x02		; float data
	ld	(_kbsave),a
	out	(c),a
.endm

.macro GET_CLOCK
	in	a,(c)		; needs to be very fast
	and	#4		; caller already deals with timeout
.endm

.macro FLOAT_CLOCK
	ld	a,(_kbsave)
	or	a,#0x01		; float the clock
	ld	(_kbsave),a
	out	(c),a
.endm

.macro LOWER_CLOCK
	ld	a,(_kbsave)
	and	a,#0xFE		; pull the clock down
	ld	(_kbsave),a
	out	(c),a
.endm

.macro LOWER_DATA
	ld	a,(_kbsave)
	and	a,#0xFD		; pull the data line down
	ld	(_kbsave),a
	out	(c),a
.endm

ps2_handlers kbd 0xBB 0xBB 255 255 kbd_bit_in kbd_bit_out

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
;	The bud 
;

kbd_bit_in:
	; Step 1: Wait for clock to go low
	call	read_port
	bit	2,a	
	jr nz,  kbd_bit_in
	and	#8
	add	d		; force carry if set
	rr	e		; into the result
	push	af		; save the carry bit for the caller
kbd_bit_in_2:
	call	read_port
	bit	2,a
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
	bit 	2,a
	jr 	nz, kbd_bit_out
	ld	a, (_kbsave)		; get the state of the other bits
	and	#0xFD
	or	#1			; we need to leave the clock floating
	rr	l
	jr	nc, kbdout0		; write a 0 bit then
	or	#2			; write a 1 bit
kbdout0:
	out	(c),a
	ld	(_kbsave),a
	; Now wait until we enter clock high
kbd_bit_out_2:
	call	read_port
	bit	2,a
	jr	z, kbd_bit_out_2
	ret
