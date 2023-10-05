; 2014-12-31 William R Sowerbutts
; DS1302 support for the N8 - same as the N8VEM Mark 4
; Need to be careful as bit 2 is owned by the SD card driver hence
; use of _rtc_shadow

        .module ds1302-n8
        .z180

        ; exported symbols
        .globl _ds1302_set_ce
        .globl _ds1302_set_clk
        .globl _ds1302_set_data
        .globl _ds1302_set_driven
        .globl _ds1302_get_data

	.globl _rtc_shadow

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

N8_RTC          = 0x88
PIN_CE          = 0x10
PIN_DATA_HIZ    = 0x20
PIN_CLK         = 0x40
PIN_DATA_OUT    = 0x80
PIN_DATA_IN     = 0x01

.area _CODE

_ds1302_get_data:
        in a, (N8_RTC)       ; read input register
        and #PIN_DATA_IN        ; mask off data pin
        ld l, a                 ; return result in L
        ret

_ds1302_set_driven:
        ld b, l                 ; load argument from caller
        ld a, (_rtc_shadow)
        and #~PIN_DATA_HIZ      ; 0 - output pin
        bit 0, b                ; test bit
        jr nz, writereg
        or #PIN_DATA_HIZ
        jr writereg

_ds1302_set_data:
        ld bc, #(((~PIN_DATA_OUT) << 8) | PIN_DATA_OUT)
        jr setpin

_ds1302_set_ce:
        ld bc, #(((~PIN_CE) << 8) | PIN_CE)
        jr setpin

_ds1302_set_clk:
        ld bc, #(((~PIN_CLK) << 8) | PIN_CLK)
        jr setpin

setpin:
        ld a, (_rtc_shadow)      ; load current register contents
        and b                   ; unset the pin
        ld b, l                 ; load argument from stack
        bit 0, b                ; test bit
        jr z, writereg          ; arg is false
        or c                    ; arg is true
writereg:
        out (N8_RTC), a      ; write out new register contents
        ld (_rtc_shadow), a      ; update our shadow copy
        ret

