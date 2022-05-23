; 2014-12-31 William R Sowerbutts
; N8VEM Mark IV SBC DS1302 real time clock interface code

        .module ds1302-mark4
        .z180

        ; exported symbols
        .globl _ds1302_set_ce
        .globl _ds1302_set_clk
        .globl _ds1302_set_data
        .globl _ds1302_set_driven
        .globl _ds1302_get_data

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

MARK4_RTC       = MARK4_IO_BASE + 0x0A
PIN_CE          = 0x10
PIN_DATA_HIZ    = 0x20
PIN_CLK         = 0x40
PIN_DATA_OUT    = 0x80
PIN_DATA_IN     = 0x01

.area _DATA

rtc_shadow:     .db 0           ; we can't read back the latch contents, so we must keep a copy

.area _CODE

_ds1302_get_data:
        in a, (MARK4_RTC)       ; read input register
        and #PIN_DATA_IN        ; mask off data pin
        ld l, a                 ; return result in L
        ret

_ds1302_set_driven:
        ld b, l                 ; load argument from caller
        ld a, (rtc_shadow)
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
        ld a, (rtc_shadow)      ; load current register contents
        and b                   ; unset the pin
        ld b, l                 ; load argument from stack
        bit 0, b                ; test bit
        jr z, writereg          ; arg is false
        or c                    ; arg is true
writereg:
        out (MARK4_RTC), a      ; write out new register contents
        ld (rtc_shadow), a      ; update our shadow copy
        ret

