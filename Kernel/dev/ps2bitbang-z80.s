;
;	PS/2 bitbang port. This is actually much easier than most
;	bitbangers as the PS/2 port provides the clocking for us
;
;	We take pains to generate port output with B no zero so that we
;	can (with care) map this over a Z180 port that is only used at
;	setup time.
;
;	TODO: longer duration errors.
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

;
;	Timeout long jump
;
timeout:
	exx
	ld hl,#0xfffd
	ld sp, (abort_sp)
	ld a,(_kbsave)
	ld bc, (_kbport)
	or #0x03		; let it all float for debug FIXME
	out (c),a
	ret

;
;	Read from Keyboard (PS/2 is different bits)
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
	ld bc,(_kbport)
kbget:
	ld (abort_sp),sp
	; Stop pulling down CLK so that the keyboard can talk
	ld a,(_kbsave)
	or #0x03		; let clock rise, don't pull data		
	out (c),a
	; Most keyboards respond within 150uS
kbwclock:
	in a,(c)
	and #4			; sample clock input
	jr z, kbdata
	djnz kbwclock
	; It didn't reply so there was no interest
	; Jam the clock again so that it can't send until we check
kbdone:
	ld hl,#0xffff
kbout:
	ld a,(_kbsave)
	; B is now zero - make it non zero so we can use high ports with
	; Z180
	inc b
	or #0x02
	out (c),a		; put the clock back down, don't pull data
	ret
	;
	; We got a rising edge. That means the keyboard wishes to talk to
	; us.
	;
kbdata:
	ld d,#0xFF	; trick so we can turn bit 2 into carry fast
	;
	; We got a clock edge, that means there is incoming data:
	; There should be a start, eight data and an odd parity
	;
	exx
	ld hl,#0		; timeout timer - FIXME value ?
	exx
	ld b,#8
	call kbdbit	; Start bit
nextbit:
	call kbdbit
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
	call kbdbit
	ld a,e		; get parity bit into A
	and #0x80	; mask other bits
	cp h
	ld h,#0
	jr z, kbout	; parity was good
	ld hl,#0xFFFE	; Parity was bad
	jr kbout
kbdbad:
	inc b
	call kbdbit	; throw away parity
	; Check stop bits ??
	ld hl,#0xFFFC		; report -err for wrong start
	jr kbout

;
;	Receive a bit. Wait for the clock to go low, sample the data and
;	then wait for it to return high. The sampled bit is added to E
;
kbdbit:
	exx
	dec hl
	ld a,h
	or l
	jr z,timeout
	exx
	in a,(c)
	bit 2,a
	jr nz, kbdbit
	; Falling clock edge, sample data is in bit 3
	and #8
	add d		; will set carry if 1
	rr e		; rotate into E
	; Wait for the rising edge
	; Preserve carry for this loop, our caller needs the carry
	; from the RL E
	push af
kbdbit2:
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
	bit 2,a
	jr z,kbdbit2
	; E now updated
	pop af
	ret

;
;	Read from PS/2
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
	ld bc,(_kbport)
ps2get:
	ld (abort_sp),sp
	; Stop pulling down CLK so that the keyboard can talk
	ld a,(_kbsave)
	or #0x0C	;	let go of clock, don't pull data
	out (c),a
	; Most keyboard respond within 150uS
ps2wclock:
	in a,(c)
	rra
	jr nc, kbdata
	djnz ps2wclock
	; It didn't reply so there was no interest
	; Jam the clock again so that it can't send until we check
ps2done:
	ld hl,#0xffff
ps2out:
	inc b
	ld a,(_kbsave)
	out (c),a
	xor a
	ret
ps2data:
	;
	; We got a clock edge, that means there is incoming data:
	; There should be a start, eight data and an odd parity
	;
	exx
	ld hl,#0		; timeout timer - FIXME value ?
	exx
	ld b,#8
	call ps2bit
ps2nextbit:
	call ps2bit
	djnz ps2nextbit
	; E now holds the data, C should be the start bit
	jr nc, ps2bad
	ld a,e
	or a
	ld h,#0x80		; For even parity of the 8bits expect a 1
	jp pe, ps2evenpar
	ld h,#0x00
ps2evenpar:
	inc b
	ld l,a		; Save the keycode
	call ps2bit
	ld a,e		; get parity bit into A
	and #0x80	; mask other bits
	cp h
	ld h,#0
	jr z, ps2out	; parity was good
	ld hl,#0xFFFE
	jr ps2out
ps2bad:
	inc b
	call ps2bit	; throw away parity
	; Check stop bits ??
	ld hl,#0xFFFC		; report -err for wrong start
	jr ps2out

ps2bit:
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
	rra
	jr c, ps2bit
	; Falling clock edge, sample data is in bit 3
	rra
	rr e		; rotate into E
	; Wait for the rising edge
ps2bit2:
	exx
	dec hl
	ld a,h
	or l
	jp z, timeout
	exx
	in a,(c)
	rra 
	jr nc, ps2bit2
	; E now updated
	ret

;
;	Send side. For an AT keyboard we also get to send it messages. Same
;	for fanicer mice to kick them into better modes
;
;	This code needs some longer timeout logic to abort if the keyboard
;	is unplugged or otherwise throws a fit
;
;	Send character L to the keyboard and return the result code back
;	where 0xFE means 'failed'. Must not mix interrupt polling of
;	keyboard with calls here.
;
;	uint8_t kbput(uint8_t c) __z88dk_fastcall
;
_ps2kbd_put:
	ld bc,(_kbport)
kbdput:
	ld (abort_sp),sp
	exx
	ld hl,#0		; timeout timer - FIXME value ?
	exx
	ld a,(_kbsave)
	and #0xFE		; Pull clock low
	or #0x02		; Keep data floating
	out (c),a		; Clock low, data floating
	; 100uS delay		- actually right now the 125uS poll delay
clkwait:
	djnz clkwait
	ld a,(_kbsave)
	ld b,#8			; Ensure B is always non zero
	out (c),a		; Clock and data low
	and #0xFC
	or #0x01		; Release clock
	out (c),a
	; No specific start bit needed ?
	ld d,l		; save character
kbdputl:
	call kbdoutbit
	djnz kbdputl
	; Check the parity bit to send
	ld a,d
	or a
	ld l,#1
	jp pe,kbdoutp1
	dec l
kbdoutp1:
	inc b
	call kbdoutbit
	ld l,#0xFF	; stop bits are 1s
	call kbdoutbit
	call kbdoutbit
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
	or #3
	inc b
	out (c),a
	; FIXME - need a general long timeout here
	; Wait for the keyboard to pull the clock low
waitk:
	in a,(c)
	and #4
	jr nz, waitk
	; Return the status code (FE = failed try again)
	jp kbdata

	;
	; Send a bit to the keyboard. The PS/2 keyboard provides the clock
	; so we wait for the clock, then send a bit, then wait for the other
	; clock edge.
	;
kbdoutbit:
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
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
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
	and #4
	jr z,kbdoutw1		; wait for clock to go back high
	ret

;
;	And mouse messages - same logic different bits
;
;	uint8_t ps2put(uint8_t c) __z88dk_fastcall
;
_ps2mouse_put:
	ld bc,(_kbport)
ps2put:
	ld (abort_sp),sp
	exx
	ld hl,#0		; timeout timer - FIXME value ?
	exx
	ld a,(_kbsave)
	and #0xFC		; clock high data low

	or #0x08
	ld b,#8
	out (c),a
	; No start bit needed ?
	ld d,l		; save character
ps2putl:
	call ps2outbit
	djnz ps2putl
	; Check the parity bit to send
	ld a,d
	or a
	ld l,#1
	jp p,ps2outp1
	dec l
ps2outp1:
	inc b
	call ps2outbit  ; clock ?
	ld l,#0xFF	; stop bits are 1s
	call ps2outbit
	call ps2outbit
	;
	; Wait 20uS
	;
	ld de,(_kbdelay)
	ld b,d
ps2del1:
	djnz ps2del1
	ld a,(_kbsave)
	inc b
	; force clock low data floating
	out (c),a
	;
	; Wait 44uS
	;
	ld b,e
ps2del2:
	djnz ps2del2
	;
	; Now we should get a reply
	;
	;
	; Raise clock and data
	;
	ld a,(_kbsave)
	or #0x0C
	inc b
	out (c),a
	; FIXME - need a general long timeout here
waitps2:
	in a,(c)
	and #4
	jr nz, waitps2
	jp ps2data

ps2outbit:
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
	rra
	jr c, ps2outbit		; wait for clock low
	rr l			; FIXME send bit order ?
	ld a,(_kbsave)		; has the bit fixed at 0 and clock not pulled
	jr nc, ps2outa
	or #8			; set data
ps2outa:
	out (c),a
ps2outw1:
	exx
	dec hl
	ld a,h
	or l
	jp z,timeout
	exx
	in a,(c)
	rra
	jr nc,ps2outw1		; wait for clock to go back high
	exx
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
