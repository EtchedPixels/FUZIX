; 2015-02-19 Sergey Kiselev
; 2014-12-31 William R Sowerbutts
; N8VEM SBC / Zeta SBC / RC2014 DS1302 real time clock interface code
;
;
        ; exported symbols
        .export _ds1302_set_ce
        .export _ds1302_set_clk
        .export _ds1302_set_data
        .export _ds1302_set_driven
        .export _ds1302_get_data

#include "../build/kernelu.def"
#include "../cpu-z80u/kernel-z80.def"

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

_ds1302_get_data:
	ld bc,(_rtc_port)
        in a, (c)       	; read input register
        and #PIN_DATA_IN        ; mask off data pin
        ld l, a                 ; return result in L
        ret

_ds1302_set_driven:
	pop de
	pop hl
	push hl
	push de
        ld b, l                 ; load argument
        ld a, (_rtc_shadow)
        and >PIN_DATA_HIZ      ; 0 - output pin
        bit 0, b                ; test bit
        jr nz, writereg
        or <PIN_DATA_HIZ
        jr writereg

_ds1302_set_data:
        ld bc, PIN_DATA_OUT
        jr setpin

_ds1302_set_ce:
        ld bc, PIN_CE
        jr setpin

_ds1302_set_clk:
        ld bc, PIN_CLK
        jr setpin

setpin:
	pop de
	pop hl
	push hl
	push de
        ld a, (_rtc_shadow)     ; load current register contents
        and b                   ; unset the pin
        ld b, l                 ; load argument from caller (fastcall)
        bit 0, b                ; test bit
        jr z, writereg          ; arg is false
        or c                    ; arg is true
writereg:
	ld bc, (_rtc_port)
        out (c), a	        ; write out new register contents
        ld (_rtc_shadow), a      ; update our shadow copy
        ret
