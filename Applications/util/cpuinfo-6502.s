	.include "zeropage.inc"

	.P816
	.I8
	.A8

.export _cpu_identify
.export _cpu_bcdtest
.export _cpu_rortest
.export _cpu_bitop
.export _cpu_ce02
.export _cpu_huc
.export _cpu_740

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

; Only called on 65C02

	.PC02

_cpu_bitop:
	stz tmp1
	smb0 tmp1	; no-op on processors without bitops
			; something else on 65C816 so don't use it there!
	lda tmp1
	rts

_cpu_ce02:
	lda #1
	.byte $43	; asr a (nop on 65C02)
	eor #1
	rts

_cpu_huc:
	ldx #1
	lda #0
	.byte $22	; no-op if a normal 65C02
	rts

;
;	On a 740 $3C is ldm. On a 65C02 it's bit $nnnn,x so we'll not
;	change tmp1
;
_cpu_740:
	lda #0				; don't STZ - no stz on a 740
	sta tmp1			; and it decodes otherwise
	.byte $3C			;ldm tmp1,2
	.byte $01
	.byte tmp1
	lda tmp1
	rts

