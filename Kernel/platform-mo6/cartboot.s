;
;	We keep all our writeable stuff in the low 8K and in the
;	discard area. We just need to write these over, do the
;	upper bank and set catridge banks to the kernel and go
;

		.org	$B000
start:
	orcc	#$10
	lds	#$61FF
	lda	#$25			; video from colour plane 40 col mono
	sta	$A7DC
	lda	#0
	sta	$A7DD			; video bank 0, black border

	lda	$A7A7			; ensure MO6/TO8 mapping is on
	ora	#$10
	sta	$A7A7

	lda	$A7C3			; map the unused colour memory
	anda	#$FE
	sta	$A7C3

	ldx	#bitstream
	ldu	#$0
low:
	ldd	,x++
	std	,u++
	cmpu	#$2000
	bne	low

	ldu	#$2200
	ldd	#0
wipe:
	std	,u++
	cmpu	#$2800
	bne	wipe

discard:
	ldd	,x++
	std	,u++
	cmpu	#$6000
	bne	discard

	jmp	$2800

		.org	$B100
bitstream:

		.org	$EFE0
	.ascii "FUZIX!"
	.byte	$04
	.ascii "01-23-45"
	.byte 0,0,0,0,0
	.byte 'X'
	.byte '"'
	.byte 0
	.byte 0
	.ascii "DCMO5"
	.word start
