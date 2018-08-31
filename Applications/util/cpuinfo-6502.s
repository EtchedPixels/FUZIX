	.include "zeropage.inc"

	.P816
	.I8
	.A8

.export _cpu_identify
.export _cpu_bcdtest
.export _cpu_rortest

_cpu_identify:
	lda #0
	sta tmp1
	inc
	cmp #1
	bmi nmos
	xba
	dec
	xba
	inc
nmos:	rts

_cpu_rortest:
	lda #$01
	ror a
	rts

_cpu_bcdtest:
	sed
	lda #$09
	clc
	adc #$01
	cld
	rts
