;
;	Boot block - runs from 2200
;	Must be under 128 bytes (byte 127 is the sum)
;
;	TODO convert to MO6
;
	org	$2200

start:	orcc	#$10		; ints off (FIR off to ?)
	lds	#$61FF
	clr	<$49		; drive 0
	clr	<$4A		; track 0
	clr	<$4B		; track 0
	lda	#2
	sta	<$4C		; sector 2

	lda	$A7A7		; force memory mapping TO8 mode
	ora	#0x10		; will need to rework this for TO9
	sta	$A7A7
	ldd	#$0462
	std	$A7E5		; bank 4 high, bank 2 low

	; Now load the sectors 256 bytes at a time
	ldx	#$6000
	bsr	load_seg
	ldx	#$B000
	bsr	load_seg
	bsr	load_most
	bsr	load_half

	; and into kernel

	jmp	$2800

load_half:
	; Switch map to the unused colour memory
	; as we are bootstrapping this setup in page1 mode
	lda	$A7C3
	anda	#$FE
	sta	$A7C3
	ldx	#$0000
	ldy	#32			; 8K at 4000
	bra	loader
load_most:
	ldx	#$2400
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
;	TODO: via SWI
;	jsr	$E82A
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
	sta	$A7DD		; flash border to show progress
	leay	-1,y
	bne	load_loop
and_rts:
	rts
failure:
	bra	failure
