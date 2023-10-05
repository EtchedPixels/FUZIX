;
;	7508 co-processor support
;
		.module cpu7508

		.area _CODE


		.globl _c7508_msg
		.globl c7508_power_off
		.globl c7508_interrupt

		.globl _rtc_clock

c7508_cmdbyte:
		; Wait for the 7508 to go ready
		in a, (0x05)
		and 4
		jr z, c7508_cmdbyte
		; Write the command byte
		ld a, c
		out (0x6), a
		; Clear the 7508 ready signal
		ld a, #2
		out (0x01), a
		ret

;
;	Wait for a byte of response from the 7508 and return it in D.
;
c7508_replybyte:
		in a, (0x05)
		and 4
		jr z, c7508_waitrsp
		in a, (0x06)
		ld d, a
		ld a, #2
		out (0x01), a
		ret

c7508_interrupt:
		; The 7508 wants us to ask it for status
		ld c, #0x02
		call c7508_cmdbyte
		call c7508_waitrsp
		in a, (0x06)
		cp #0xC0
		jr c, c7508_keyin
		bit 5, a
		call nz, c7508_1sec
		bit 4, a
		call nz, c7508_z80reset
		bit 3, a
		call nz, c7508_7508reset
		bit 2, a
		call nz, c7508_powerfail
		bit 1, a
		call nz, c7508_alarm
		bit 0, a
		ret z
		; power switch
		; FIXME
		ret
c7508_alarm:
		ret
c7508_powerfail:
		ret
c7508_7508reset:
		ret
c7508_z80reset:
		ret
c7508_1sec:
		push af
		ld hl, _rtc_clock
		inc (hl)
		pop af
		ret

;
;	Keyboard event - punt out to C
;
c7508_keyin:
		ld l, a
		ld h, #0
		push hl
		push af
		call _key_pressed
		pop af
		pop hl
		ret

;
;	Turn off the 7508 as an interrupt source
;
c7508_irqoff:
		ld a, #0x0a
		out (4), a
		ret

;
;	Turn on the 7508 as an interrupt source
;
c7508_irqon:
		ld a, #0x0b
		out (4), a
		ret
;
;	Send a command. HL points to the start of the command B holds the
;	number of bytes to send, the reply will be placed in the memory
;	following the command and E bytes are expected
;
c7508_docmd:
		call c7508_irqoff
		ld c, (hl)
		inc hl
		call c7508_cmdbyte
		djnz c7508_docmd
		ld b, e
		xor a
		cp b
		jp z, c7508_irqon
c7508_doreply:
		call c7508_replybyte
		ld (hl), d
		djnz c7508_doreply
		call c7508_irqon
		ret

_c7508_power_off:
		ld c, #1
		call c7508_cmdbyte		; won't return !
		di
		hlt

;
;	General purpose C interface to 7508 commands. Pass a buffer that
;	holds the length to send, length to receive, send data, space for
;	receive data.
;
_c7508_msg:
		pop bc
		pop de
		pop hl			; Parameter
		push hl
		push de
		push bc
		ld b, (hl)
		inc hl
		ld e,(hl)
		inc hl
		jr c7508_docmd

		.area _DATA
_rtc_clock:	.db 0
