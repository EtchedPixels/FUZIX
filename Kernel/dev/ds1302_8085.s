#include "../kernel-8080.def"
!
!	8085 version of the DS1302 support for RC2014
!

#define RTCREG		0x0C

#define PIN_DATA_IN	0x01
#define PIN_CE		0x10
#define PIN_DATA_HIZ	0x20
#define PIN_CLK		0x40
#define PIN_DATA_OUT	0x80

#define PIN_DATA_MASK	0x7F00
#define PIN_CE_MASK	0xEF00
#define PIN_CLK_MASK	0xBF00

	.sect .text

! -----------------------------------------------------------------------------
! DS1302 interface
! -----------------------------------------------------------------------------

	.define _ds1302_get_pin_data

_ds1302_get_pin_data:
	in RTCREG		! read input register
        ani PIN_DATA_IN        ! mask off data pin
        mov e, a                ! return result in L
	mvi d, 0
        ret

	.define _ds1302_set_pin_data_driven

_ds1302_set_pin_data_driven:
	ldsi 2
	lhlx
        mov a, l		! load argument
	ani 1
        lda _rtc_shadow
	jz setreg
        ani ~PIN_DATA_HIZ	! 0 - output pin
        jmp writereg
setreg:
        ori PIN_DATA_HIZ
        jmp writereg

	.define _ds1302_set_pin_data

_ds1302_set_pin_data:
	push b
        lxi b, PIN_DATA_OUT + PIN_DATA_MASK
        jmp setpin

	.define _ds1302_set_pin_ce

_ds1302_set_pin_ce:
	push b
        lxi b, PIN_CE + PIN_CE_MASK
        jmp setpin

	.define _ds1302_set_pin_clk

_ds1302_set_pin_clk:
	push b
        lxi b, PIN_CLK + PIN_CLK_MASK
	! Fall through
setpin:
	ldsi 4
	lhlx			! Argument into HL
	mov a, l		! Are we setting or clearing ?
	ora a
	jnz set
	mov c, a		! A is 0 so set C to 0 so clears pin
set:
        lda _rtc_shadow		! load current register contents
        ana b                   ! unset the pin
        ora c			! set if arg is true
	pop b
writereg:
	out RTCREG		! write out new register contents
        sta _rtc_shadow		! update our shadow copy
        ret
