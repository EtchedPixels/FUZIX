;
;	Boot block - runs from 6200
;	Must be under 128 bytes (byte 127 is the sum)
;
	org	$6200

start:	orcc	#$10		; ints off (FIR off to ?)
	lds	#$61FF
	clr	<$49		; drive 0
	clr	<$4A		; track 0
	clr	<$4B		; track 0
	lda	#2
	sta	<$4C		; sector 2

	;	Force on new style banking
	;	This is ugly - we shouldn't be hard coding this but
	;	presumably finding it somewhere
	lda	#$54
	sta	$E7E7
	ldd	#$0462
	std	$E7E5		; bank 4 high, bank 2 low

	; Now load the sectors 256 bytes at a time
	ldx	#$A000
	bsr	load_seg
	ldx	#$0000
	bsr	load_seg
	bsr	load_most
	bsr	load_half

	; and into kernel

	jmp	$6800

load_half:
	; Switch map to the unused colour memory
	; as we are bootstrapping this setup in page1 mode
	lda	$E7C3
	anda	#$FE
	sta	$E7C3
	ldx	#$4000
	ldy	#32			; 8K at 4000
	bra	loader
load_most:
	ldx	#$6400
	ldy	#60
	bra	loader
load_seg:
	ldy	#64
loader:
	; Load Y sectors starting at X
	stx	<$4F
load_loop:
	ldb	#2
	stb	<$48
	jsr	$E82A
	bcs	failure
	inc	<$4F		; move on 256 bytes
	lda	<$4C
	cmpa	#16		; sector on
	bne	track_same
	inc	<$4B		; track on, sector back to 1
	clr	<$4C		; will drop through and inc to 1
track_same:
	inc	<$4C
	anda	#$0F
	sta	$E7DD		; flash border to show progress
	leay	-1,y
	bne	load_loop
and_rts:
	rts
failure:
	bra	failure
