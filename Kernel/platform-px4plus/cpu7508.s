;
;	7508 co-processor support
;
		.module cpu7508

		.area _CODE

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

c7508_waitrsp:
		in a, (0x05)
		and 4
		jr z, c7508_waitrsp
		ret

c7508_int:	; The 7508 wants us to ask it for status
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

c7508_keyin:
		; FIXME
		ret

		.area _DATA
_rtc_clock:	.db 0
