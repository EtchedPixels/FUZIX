;
;	The bit bang support routines for the RC2014 platform
;

	.globl _ps2kbd_get
	.globl _ps2kbd_put
	.globl _ps2mouse_get
	.globl _ps2mouse_put
	.globl _ps2kbd_beep

	.globl ps2_read_port
	.globl ps2_receive_code
	.globl ps2_send_code

	.globl _kbsave

	.area _CODE


_ps2mouse_get:
	ld	hl,#rc2014_mouse
	jp	ps2_receive_code

_ps2kbd_get:
	ld	hl,#rc2014_keyboard
	jp	ps2_receive_code

_ps2mouse_put:
	ld	a,l
	ld	hl,#rc2014_mouse
	jp	ps2_send_code

_ps2kbd_put:
	ld	a,l
	ld	hl,#rc2014_keyboard
	jp	ps2_send_code

rc2014_mouse:
	.byte	0xBB		; 	port
	.byte	0xFF		;	delay (should be 100us or more)
	.byte	0xFF		;	excessive - tune

	.word	mouse_bit_in
	.word	mouse_bit_out
	.word	mouse_hold_off
	.word	mouse_get_clock
	.word	mouse_float_clock
	.word	mouse_lower_clock
	.word	mouse_lower_data

rc2014_keyboard:
	.byte	0xBB		; 	port
	.byte	0xFF		;	delay (should be 100us or more)
	.byte	0xFF		;	excessive - tune

	.word	kbd_bit_in
	.word	kbd_bit_out
	.word	kbd_hold_off
	.word	kbd_get_clock
	.word	kbd_float_clock
	.word	kbd_lower_clock
	.word	kbd_lower_data

_ps2kbd_beep:

	ld	a, (rc2014_mouse)
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
;	Simple functions to control the lines. C will hold the port in the
;	structure so is ready for us. A is free for our use, other registers
;	must be saved if changed
;

;
;	Mouse: clock on bit 2 data on bit 3, low is pulled down
;	clock input is on bit 0
;
mouse_hold_off:			; jam clock, float data
	ld	a,(_kbsave)
	and	a,#0xFB		; force clock down
	or	a,#0x08		; float data
set_port:
	ld	(_kbsave),a
	out	(c),a
	ret

mouse_get_clock:
	call	ps2_read_port
	and	#1
	ret

mouse_float_clock:
	ld	a,(_kbsave)
	or	a,#0x04		; float clock
	jr	set_port

mouse_lower_clock:
	ld	a,(_kbsave)
	and	a,#0xFB		; pull clock down
	jr	set_port

mouse_lower_data:
	ld	a,(_kbsave)
	and	a,#0xF7		; pull data down
	jr	set_port

;
;	Keyboard: clock on bit 0 data in bit 1, low is pulled down
;	clock input is on bit 2
;
kbd_hold_off:
	ld	a,(_kbsave)
	and	a,#0xFE		; force clock down
	or	a,#0x02		; float data
	jr	set_port

kbd_get_clock:
	call	ps2_read_port
	and	#4
	ret

kbd_float_clock:
	ld	a,(_kbsave)
	or	a,#0x01		; float the clock
	jr	set_port

kbd_lower_clock:
	ld	a,(_kbsave)
	and	a,#0xFE		; pull the clock down
	jr	set_port

kbd_lower_data:
	ld	a,(_kbsave)
	and	a,#0xFD		; pull the data line down
	jr	set_port

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
;	IX is a general purpose pointer to the data block for the port
;	IY will hold the function itself
;	C is PORT(IX)
;	D is 0xFF (for converting bits to carry)
;	E is the data on receive
;	L is the data on send
;
;	IX, IY, B, E, H, L should be preserved or modified as stated
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
	call	ps2_read_port
	bit	2,a	
	jr nz,  kbd_bit_in
	and	#8
	add	d		; force carry if set
	rr	e		; into the result
	push	af		; save the carry bit for the caller
kbd_bit_in_2:
	call	ps2_read_port
	bit	2,a
	jr nz,	kbd_bit_in_2
	pop	af		; recover carry
	ret
;
;	Same for the mouse but we can shortcut the logic a bit as
;	the bit we want is in bit 1 and the clock bit 0
;
mouse_bit_in:
	call 	ps2_read_port
	rra 
	jr c,	mouse_bit_in
	rra
	rr e			; rotate data bit into E
	push	af		; preserve carry
mouse_bit_in_2:
	call 	ps2_read_port
	rra
	jr 	nc, mouse_bit_in_2
	pop	af
	ret

;
;	The transmit protocol is basically the same except that we are the
;	one who puts a bit on the bus during low clock and the device
;	samples it on the high clock
;
kbd_bit_out:
	call	ps2_read_port
	bit 	2,a
	jr 	nz, kbd_bit_out
	ld	a, (_kbsave)		; get the state of the other bits
	or	#1			; we need to leave the clock floating
	rr	l
	jr	nc, kbdout0		; write a 0 bit then
	or	#2			; write a 1 bit
kbdout0:
	out	(c),a
	; Now wait until we enter clock high
kbd_bit_out_2:
	call	ps2_read_port
	bit	2,a
	jr	z, kbd_bit_out_2
	ret

mouse_bit_out:
	call	ps2_read_port
	rra 
	jr 	c, kbd_bit_out
	ld	a, (_kbsave)		; get the state of the other bits
	or	#1			; we need to leave the clock floating
	rr	l
	jr	nc, mouseout0		; write a 0 bit then
	or	#2			; write a 1 bit
mouseout0:
	out	(c),a
	; Now wait until we enter clock high
mouse_bit_out_2:
	call	ps2_read_port
	rra
	jr	nc, mouse_bit_out_2
	ret

	.area	_DATA
_kbsave:
	.byte	0
