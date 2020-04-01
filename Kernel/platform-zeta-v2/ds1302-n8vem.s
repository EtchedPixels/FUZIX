; 2015-02-19 Sergey Kiselev
; 2014-12-31 William R Sowerbutts
; N8VEM SBC / Zeta SBC DS1302 real time clock interface code
;
;	FIXME: belongs in dev/
;

        .module ds1302-n8vem

        ; exported symbols
        .globl _ds1302_set_pin_ce
        .globl _ds1302_set_pin_clk
        .globl _ds1302_set_pin_data
        .globl _ds1302_set_pin_data_driven
        .globl _ds1302_get_pin_data

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

N8VEM_RTC       = 0x70
PIN_CE          = 0x10
PIN_DATA_HIZ    = 0x20
PIN_CLK         = 0x40
PIN_DATA_OUT    = 0x80
PIN_DATA_IN     = 0x01

.area _DATA

rtc_shadow:     .db 0           ; we can't read back the latch contents, so we must keep a copy

.area _CODE

_ds1302_get_pin_data:
        in a, (N8VEM_RTC)       ; read input register
        and #PIN_DATA_IN        ; mask off data pin
        ld l, a                 ; return result in L
        ret

_ds1302_set_pin_data_driven:
        ld b, l                 ; load argument from stack
        ld a, (rtc_shadow)
        and #~PIN_DATA_HIZ      ; 0 - output pin
        bit 0, b                ; test bit
        jr nz, writereg
        or #PIN_DATA_HIZ
        jr writereg

_ds1302_set_pin_data:
        ld bc, #(((~PIN_DATA_OUT) << 8) | PIN_DATA_OUT)
        jr setpin

_ds1302_set_pin_ce:
        ld bc, #(((~PIN_CE) << 8) | PIN_CE)
        jr setpin

_ds1302_set_pin_clk:
        ld bc, #(((~PIN_CLK) << 8) | PIN_CLK)
        jr setpin

setpin:
        ld a, (rtc_shadow)      ; load current register contents
        and b                   ; unset the pin
        ld b, l                 ; load argument from stack
        bit 0, b                ; test bit
        jr z, writereg          ; arg is false
        or c                    ; arg is true
writereg:
        out (N8VEM_RTC), a      ; write out new register contents
        ld (rtc_shadow), a      ; update our shadow copy
        ret

