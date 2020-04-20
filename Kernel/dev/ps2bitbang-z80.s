;
;	PS/2 bitbang port. This is actually much easier than most
;	bitbangers as the PS/2 port provides the clocking for us
;
;	We take pains to generate port output with B no zero so that we
;	can (with care) map this over a Z180 port that is only used at
;	setup time.
;
;	2ms: longest a send should ever take
;	15ms: longest we should ever have to wait for clocks when we ask
;	to tx
;	20ms: longest a command reply should take
;
;	Assumes <= 8MHz processor (so 125uS is 1000 clocks)
;

	.include "kernel.def"
	.include "../kernel-z80.def"

	.module ps2bitbang

	.globl _ps2kbd_get
	.globl _ps2mouse_get
	.globl _ps2kbd_put
	.globl _ps2mouse_put

	.globl _ps2kbd_beep

	.globl _kbsave
	.globl _kbdelay
	.globl _kbport
	.globl _kbwait

	.globl outchar
	.globl outcharhex
;
;	This isn't ideal. What we should do once it all works is turn on
;	the PS/2 and Keyboard, poll them both and run a state machine so
;	we can receive from both at once asynchronously. Ugly code to write
;	so make it all work first!
;

	.area _CODE

TIMEOUT		.equ	0	; 65536 iterations - could be tuned but
				; seems fine as is.

CLOCKONLY	.equ -1
CLOCKIN		.equ -2
CLOCKDATA 	.equ -3

;
;	Timeout long jump
;
timeout:
	exx
	ld hl,#0xfffd
	ld sp, (abort_sp)
	ld a,(_kbsave)
	ld bc, (_kbport)
	or CLOCKONLY(iy)
	out (c),a		; jam the clock
	pop iy			; our callers all leave the old IY on
	ret			; the frame for us to recover


;
;	Read from PS/2 Mouse
;
;	A = scratch throughout
;	C = port, B = loops to do for timeout
;	E = code being assembled
;	HL = return
;
;	Except for not needing D for the carry conversion this is the same
;	as the keyboard but different bit numbers
;
_ps2mouse_get:
	push iy
	ld iy,#ps2bit
	jr ps2get

;
;	Read from Keyboard (Keyboard and mouse are different bits)
;
;	A = scratch throughout
;	C = port, B = loops to do for timeout
;	D = internal kbdbit helper E = code being assembled
;	HL = return
;
;	kbsave holds the audio bits and 0x5 for the low nibble
;	(that is clocks down, data floating)
;
_ps2kbd_get:
	push iy
	ld iy,#kbdbit
;
;	Shared code entry point. IY is used to index all the per port
;	parameters
;
ps2get:
	ld bc,(_kbport)
kbget:
	; abort_sp is used for the timeout case
	ld (abort_sp),sp
	; Stop pulling down CLK so that the device can talk
	ld a,(_kbsave)
	or CLOCKDATA(iy)	; let clock rise, don't pull data
	out (c),a

	;
	; With the clock floating the device is allowed to talk to us
	; if it wants. Most devices respond within 150us
	;
	; This loop is 50 clocks per non detect
kbwclock:
	in a,(c)
	and CLOCKIN(iy)		; sample clock input
	jr z, kbdata
	djnz kbwclock
	;
	; It didn't reply so there was no interest
	; Jam the clock again so that it can't send until we check
	;
kbdone:
	ld hl,#0xffff
kbout:
	ld a,(_kbsave)
	; B is now zero - make it non zero so we can use high ports with
	; Z180
	inc b
	or CLOCKONLY(iy)
	out (c),a		; put the clock back down, don't pull data
	pop iy
	ret

	;
	; We got a rising edge. That means the device wishes to talk to
	; us.
	;
kbdata:
	ld d,#0xFF	; trick so we can turn bit 2 into carry fast
	;
	; We got a clock edge, that means there is incoming data:
	; There should be a start, eight data and an odd parity
	;
	exx
	ld hl,#TIMEOUT
	exx
	ld b,#8
	call jpiy	; Start bit
nextbit:
	call jpiy
	djnz nextbit
	; E now holds the data, carry should be the start bit
;	jr nc, kbdbad
	ld a,e
	or a		; Generate parity flag
	ld h,#0x80	; For even parity of the 8bits send a 1 to get odd
	jp pe, kbdevenpar
	ld h,#0x00	; If we are odd parity send a 0 so we stay odd
kbdevenpar:
	ld l,a		; Save the keycode
	inc b		; make sure b is non zero for Z180 ports
	call jpiy
	ld a,e		; get parity bit into A
	and #0x80	; mask other bits
	cp h
	ld h,#0
	jr z, kbrok	; parity was good
	ld hl,#0xFFFE	; Parity was bad
	jr kbout
	;
	; We didn't get a start bit, god knows what is going on
	;
kbdbad:
	ld hl,#0xFFFC	; report -err for wrong start
	jr kbout
	;
	; Wait for the stop bit. We must do this otherwise the device may
	; think we didn't receive the data. Only after we have accepted the
	; stop bit can we raise the clock to halt transmission.
	;
kbrok:
	call jpiy	; throw away the stop bit
	jr kbout

;
;	Helper. Read from the I/O port into A whilst managing the timeout
;	counter
;
inbits:
	exx
	dec hl
	ld a,h
	or l
	jp z, timeout	; longjmp out
	exx
	in a,(c)
	ret

;
;	Receive a bit. Wait for the clock to go low, sample the data and
;	then wait for it to return high. The sampled bit is added to E
;
;
;	Three bytes before function pointer are the config
;
	.byte 0x03	; Keyyboard clock and data pull down
	.byte 0x04	; Keyboard clock in
	.byte 0x02	; Keyboard clock pull down only
kbdbit:
	call inbits
	bit 2,a
	jr nz, kbdbit
	; Falling clock edge, sample data is in bit 3
	and #8
	add d		; will set carry if 1
	rr e		; rotate into E
	; Wait for the rising edge
	; Preserve carry for this loop, our caller needs the carry
	; from the RR E
	push af
kbdbit2:
	call inbits
	bit 2,a
	jr z,kbdbit2
	; E now updated
	pop af
	ret

;
;	Three bytes before function pointer are the config
;
	.byte 0x0C	; Mouse clock and data pull down
	.byte 0x01	; Mouse clock in
	.byte 0x08	; Mouse clock pull down only
ps2bit:
	call inbits
	rra		; waiting for clock to go low
	jr c, ps2bit
	; Falling clock edge, sample data is in bit 1
	rra
	rr e		; rotate into E
	; Wait for the rising edge
	; Preserve carry for this loop, our caller needs the carry
	; from the RR E
	push af
ps2bit2:
	call inbits
	rra 
	jr nc, ps2bit2	; wait for bit to go high again
	; E now updated
	pop af
	ret

;
;	Send side. For an AT keyboard we also get to send it messages. Same
;	for fanicer mice to kick them into better modes
;
;	Send character L to the keyboard and return the result code back
;	where 0xFE means 'failed'. Must not mix interrupt polling of
;	keyboard with calls here.
;
;	uint8_t _ps2kbd_put(uint8_t c) __z88dk_fastcall
;
_ps2kbd_put:
	ld bc,(_kbport)
	push iy
	ld iy,#kbdoutbit
	ld (abort_sp),sp
	call ps2_put
	ld iy,#kbdbit
	jp kbdata
;
;	And mouse messages - same logic different bits
;
;	uint8_t ps2put(uint8_t c) __z88dk_fastcall
;
_ps2mouse_put:
	ld bc,(_kbport)
	push iy
	ld iy,#ps2outbit
	ld (abort_sp),sp
	call ps2_put
	ld iy,#ps2bit
	jp kbdata

CLOCKMASK	.equ -1
CLOCKLOW	.equ -2
CLOCKREL	.equ -3
FLOATBOTH	.equ -4
DATABIT		.equ -5

ps2_put:
	exx
	ld hl,#TIMEOUT
	exx
	ld a,(_kbsave)
	and CLOCKMASK(iy)	; Pull clock low
	or CLOCKLOW(iy)		; Keep data floating
	out (c),a		; Clock low, data floating
	; 100uS delay		- actually right now the 125uS poll delay
clkwait:
	djnz clkwait
	ld a,(_kbsave)
	ld b,#8			; Ensure B is always non zero
	out (c),a		; Clock and data low
	and CLOCKMASK(iy)
	or CLOCKREL(iy)		; Release clock
	out (c),a
	; No specific start bit needed ?
	ld d,l		; save character
kbdputl:
	call jpiy
	djnz kbdputl
	; Check the parity bit to send
	ld a,d
	or a
	ld l,#1
	jp pe,kbdoutp1
	dec l
kbdoutp1:
	inc b
	call jpiy
	ld l,#0xFF	; stop bits are 1s
	call jpiy
	call jpiy
	;
	; Wait 20uS
	;
	ld de,(_kbdelay)
	ld b,d
del1:	djnz del1
	ld a,(_kbsave)
	; force clock low, data floating
	out (c),a
	;
	; Wait 44uS
	;
	ld b,e
del2:	djnz del2
	;
	; Now we should get a reply
	;
	;
	; Raise clock and data
	;
	or FLOATBOTH(iy)
	inc b
	out (c),a
	; FIXME - need a general long timeout here
	; Wait for the keyboard to pull the clock low
waitk:
	in a,(c)
	and DATABIT(iy)
	jr nz, waitk
	; Return the status code (FE = failed try again)
	ret

;
; Send a bit to the keyboard. The PS/2 keyboard provides the clock
; so we wait for the clock, then send a bit, then wait for the other
; clock edge.
;
; Table heads the function
;
;
	.byte 0x04		; data bit
	.byte 0x03		; float clock and data
	.byte 0x01		; release clock
	.byte 0x02		; clock low
	.byte 0xfe		; clock mask

kbdoutbit:
	call inbits
	and #4
	jr nz, kbdoutbit	; wait for clock low
	ld a,(_kbsave)		;
	or #1			; clock floating 
	rr l
	jr nc, kbdouta
	or #2			; set data
kbdouta:
	out (c),a
kbdoutw1:
	call inbits
	and #4
	jr z,kbdoutw1		; wait for clock to go back high
	ret


	.byte 0x02		; data bit
	.byte 0x0C		; float clock and data
	.byte 0x04		; release clock
	.byte 0x08		; clock low
	.byte 0xf3		; clock mask
ps2outbit:
	call inbits
	rra
	jr c, ps2outbit		; wait for clock low
	ld a,(_kbsave)		; has the bit fixed at 0 and clock not pulled
	or #4			; clock floating
	rr l			; FIXME send bit order ?
	jr nc, ps2outa
	or #8			; set data
ps2outa:
	out (c),a
ps2outw1:
	call inbits
	rra
	jr nc,ps2outw1		; wait for clock to go back high
	ret

_ps2kbd_beep:

	ld bc,(_kbport)
	ld b,#0
	ld de,#150

beeper:
	ld a,#0xF0
	out (c),a
wait1:
	ex (sp),hl		; These happen an even number of times
	djnz wait1
	xor a
	out (c),a
wait0:
	ex (sp),hl
	djnz wait0
	dec de
	ld a,d
	or e
	jr nz, beeper
	ret

;	Used to create a call (iy)
jpiy:	jp (iy)

	.area _DATA
_kbsave:
	.db 0x00		; Upper bit value to preserve
				; low bits set for clock zero, data float
_kbdelay:
	.dw 0x00		; delay times (20/40uS)
_kbport:
	.db 0x00		; I/O port
_kbwait:
	.db 0x00		; loops per poll (must be byte after port)
abort_sp:
	.dw 0x00
