
	.module	sdcard

	.globl _sd_spi_raise_cs
	.globl _sd_spi_lower_cs
	.globl _sd_spi_clock
	.globl _sd_spi_receive_byte
	.globl _sd_spi_transmit_byte
	.globl _sd_spi_receive_sector
	.globl _sd_spi_transmit_sector

	.globl _blk_op

PRA	equ	0xFE68

;
;	We can receive fastest on bit 7 or 0, but we want
;	bit 0 to be the clock so we can inc/dec the clock
;
DATAIN	equ	0x80
CS0	equ	0x04
DATA	equ	0x02
CLOCK	equ	0x01
;
;	This assumes that port A was properly configured
;	with the right bits set for R/W
;
;	TODO: routine variants to avoid the extra dp setup and call
;	overhhead etc on block read/writes
;

	.area	.data
spibits:
	.byte	0

	.area	.text

_sd_spi_raise_cs:
	lda	spibits
	ora	#CS0
	sta	spibits
	sta	PRA
	rts
_sd_spi_lower_cs:
	lda	spibits
	anda	#~CS0
	sta	spibits
	sta	PRA
_sd_spi_clock:			; We don't do anything fast enough
				;  to need a speed setting
	rts
_sd_spi_transmit_sector:
	pshs	y,dp
	lda	#0xFE
	tfr	a,dp
	ldx	_blk_op
	tfr	x,y
	leax	512,x
	stx	endp
	lda	_blk_op+2
	beq	writebyte
	deca
	beq	writeuser
	jsr	map_for_swap
	bra	writebyte
writeuser:
	jsr	map_process_always
writebyte:
	bsr	_sd_spi_receive_byte
	stb	,y+
	cmpy	endp
	bne	readbyte
	jsr	map_kernel
	puls	y,dp,pc
_sd_spi_receive_sector:
	pshs	y,dp
	lda	#0xFE
	tfr	a,dp
	ldx	_blk_op
	tfr	x,y
	leax	512,x
	stx	endp
	lda	_blk_op+2
	beq	readbyte
	deca
	beq	readuser
	jsr	map_for_swap
	bra	readbyte
readuser:
	jsr	map_process_always
readbyte:
	bsr	_sd_spi_receive_byte
	stb	,y+
	cmpy	endp
	bne	readbyte
	jsr	map_kernel
	puls	y,dp,pc

;
;	We pay a small cost here to use bit 0 for the clock
;	but it speeds up receive
;
_sd_spi_transmit_byte:
	pshs	dp
	lda	#$FE
	tfr	a,dp
	lda	#8
	pshs	a
	lda	spibits
writebit:
	rora			; Shift the configured bits right
	rora
	rolb			; Get the next bit to send into C
	rola			; Insert data bit into A
	lsla			; Clock clear (low bit now 0)
	sta	<PRA		; Clock low
	inca
	sta	<PRA		; Clock high
	dec	,s
	bne	writebit
	puls	a,dp,pc

_sd_spi_receive_byte:
	pshs	dp
	lda	#$FE
	tfr	a,dp
	lda	spibits
	ora	#DATA

	;	Bang out FF as fast as we can and collect the
	;	bits back
	sta	<PRA		; Low
	inc	<PRA		; High
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	dec	<PRA
	inc	<PRA
	lda	<PRA		; Sample
	rola
	rolb
	clra
	dec	<PRA
	puls	dp,pc

	.area	.data
endp:	.word	0
