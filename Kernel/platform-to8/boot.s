;
;	Boot block - runs from 6200
;	Must be under 128 bytes (byte 127 is the sum)
;
	org	$6200

start:
	orcc	#$10		; ints off (FIR off to ?)
	lds	#$61FF
	ldu	#$6000
	clr	$49,u		; drive 0
	clr	$4A,u		; track 0
	clr	$4B,u		; track 0
	lda	#2
	sta	$4C,u		; sector 2

	; Video is still as the ROM expects
	ldx	#banner
	bsr	puts

	lda	$E7E7		; force memory mapping TO8 mode
	ora	#0x10		; will need to rework this for TO9
	sta	$E7E7

	; Now load the sectors 256 bytes at a time
	lda	#4
	bsr	load_seg
	lda	#2
	bsr	load_seg
	bsr	load_most
	bsr	load_half

	; and into kernel

	jmp	$6800

load_half:
	; Switch map to the unused colour memory
	; as we are bootstrapping this setup in page1 mode
	lda	$E7C3
	ora	#$01
	sta	$E7C3
	ldx	#$4000
	ldy	#32			; 8K at 4000
	bra	loader
load_most:
	ldx	#$6400
	ldy	#60
	bra	loader
load_seg:
	; Set A000-DFFF range (assumes 8/9+ FIXME)
	sta	$E7E5
	ldx	#$A000
	ldy	#64
loader:
	; Load Y sectors startign at X
	pshs	x,y,u
	ldb	#2
	jsr	$E82A
	puls	x,y,u
	bcs	failure
	inc	$4E,u		; move on 256 bytes
	lda	$4C,u
	cmpa	#16		; sector on
	bne	track_same
	inc	$4B,u		; track on, sector back to 1
	clr	$4C,u		; will drop through and inc to 1
track_same:
	inc	$4C,u
	anda	#$0F
	sta	$E7DD		; flash border to show progress
	leay	-1,y
	bne	loader
and_rts:
	rts

puts:
	ldb	,x+
	beq	and_rts
	jsr	$E803
	bra	puts

banner:
	.byte	$1B
	.byte	$48		; Page 1 mode (not T7/70)
	.byte	$0C		; clear screen
	.ascii  'Booting...'
	.byte	0		; end
; For now
failure:
	bra	failure
