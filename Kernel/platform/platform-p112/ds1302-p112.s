; 2015-01-01 William R Sowerbutts
; DX-Designs P112 SBC DS1202 real time clock interface code

        .module ds1302-p112
        .z180

        ; exported symbols
        .globl _ds1302_set_ce
        .globl _ds1302_set_clk
        .globl _ds1302_set_driven
        .globl _ds1302_set_data
        .globl _ds1302_get_data

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; DS1202 interface
; -----------------------------------------------------------------------------

PIN_DATA        = 0x01  ; PA0
PIN_CLK         = 0x02  ; PA1
PIN_CE          = 0x04  ; PA2

.area _CODE

_ds1302_get_data:
        in0 a, (PORT_A_DATA)    ; read IO pins
        and #PIN_DATA           ; mask off data bit
        ld l, a                 ; return result in L
        ret

_ds1302_set_driven:
        in0 a, (PORT_A_DDR)     ; a 1 in each bit makes the corresponding pin tristate (input), 0 makes it an output
        and #~(PIN_DATA|PIN_CE|PIN_CLK) ; bits to 0 -> set all pins as outputs
        ld b, l                 ; load argument
        bit 0, b                ; test bit
        jr nz, writeddr         ; nonzero -> we want an output
        or #PIN_DATA            ; zero -> we want an input; change bit to 1 -> set data pin as input
writeddr:
        out0 (PORT_A_DDR), a    ; update data direction register
        ret

_ds1302_set_data:
        ld bc, #(((~PIN_DATA) << 8) | PIN_DATA)
        jr setpin

_ds1302_set_ce:
        ld bc, #(((~PIN_CE) << 8) | PIN_CE)
        jr setpin

_ds1302_set_clk:
        ld bc, #(((~PIN_CLK) << 8) | PIN_CLK)
        jr setpin

setpin:
        in0 a, (PORT_A_DATA)    ; load current register contents
        and b                   ; unset the pin
        ld b, l                 ; load argument
        bit 0, b                ; test bit
        jr z, writereg          ; arg is false, bit is already 0
        or c                    ; arg is true, set bit to 1
writereg:
        out0 (PORT_A_DATA), a   ; write out new register contents
        ret

