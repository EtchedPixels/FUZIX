#include "../kernel-68000.def"

		.globl	ds1302_get_data
		.globl	ds1302_set_driven
		.globl	ds1302_set_data
		.globl	ds1302_set_ce
		.globl	ds1302_set_clk

#define PIN_CE		$10
#define PIN_DATA_HIZ	$20
#define PIN_CLK		$40
#define PIN_DATA_OUT	$80
#define PIN_DATA_IN	$01
#define PIN_OTHER	$00

#define RTCPORT		$1000C

.mri 1

ds1302_get_data:
	moveq #0,d0
	move.b RTCPORT,d0
	and.b #PIN_DATA_IN,d0
	rts

ds1302_set_driven:
	move.b rtc_shadow,d0
	andi.b #~PIN_DATA_HIZ,d0
	btst.b #0,7(sp)
	bne.s writereg
	ori.b #PIN_DATA_HIZ,d0
	bra.s writereg


ds1302_set_data:
	move.b #~PIN_DATA_OUT,d1
	bra.s setpin

ds1302_set_ce:
	move.b #~PIN_CE,d1
	bra.s setpin

ds1302_set_clk:
	move.b #~PIN_CLK,d1
setpin:
	move.b rtc_shadow,d0
	and.b d1,d0
	btst.b #0,7(sp)
	beq writereg
	eor.b #$ff,d1
	or.b d1,d0
writereg:
	move.b d0,rtc_shadow
	move.b d0,RTCPORT
	rts
