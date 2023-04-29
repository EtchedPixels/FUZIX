#include "kernel.def"

		.file "ds1302_68hc11"
		.mode mshort
;
;	68HC11 version of the DS1302 support for RC2014
;

#define RTCREG		0xFE0C

#define PIN_DATA_IN	0x01
#define PIN_CE		0x10
#define PIN_DATA_HIZ	0x20
#define PIN_CLK		0x40
#define PIN_DATA_OUT	0x80

#define PIN_DATA_MASK	0x7F00
#define PIN_CE_MASK	0xEF00
#define PIN_CLK_MASK	0xBF00

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

	.globl ds1302_get_data

ds1302_get_data:
	ldab RTCREG		; read input register
	clra
        andb #PIN_DATA_IN        ; mask off data pin
        rts

	.globl ds1302_set_driven

ds1302_set_driven:
        ldaa rtc_shadow
        anda #~PIN_DATA_HIZ	; 0 - output pin
	andb #1
	bne writereg
        oraa #PIN_DATA_HIZ
writereg:
	staa RTCREG
	staa rtc_shadow
	rts

	.globl ds1302_set_data

ds1302_set_data:
        ldx #PIN_DATA_OUT + PIN_DATA_MASK
        bra setpin

	.globl ds1302_set_ce

ds1302_set_ce:
        ldx #PIN_CE + PIN_CE_MASK
        bra setpin

	.globl ds1302_set_clk

ds1302_set_clk:
        ldx #PIN_CLK + PIN_CLK_MASK
	; Fall through
setpin:
	clra
	xgdx			; X is now the argument D the masks
        anda rtc_shadow		; current & mask
	cpx #0
	beq writereg
	stab *_.tmp		; no oraa b
        oraa *_.tmp              ; set if relevant
	bra writereg
