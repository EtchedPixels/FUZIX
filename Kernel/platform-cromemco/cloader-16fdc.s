	.area ASEG(ABS)
	.org  0x80
;
;	We have 128 bytes and are loaded by the ROM at 0x0080. The rest is
;	bascially our problem
;
;	Load sectors 2-26 ( a whopping 3.2K )
;
;	Assumes an 8" disk in SD mode
;
	ld a,#1
	out (0x40),a		; ROM off

	ld hl,#go
	call strout

restart:
	ld bc,#0x020F		; sector 2 , restore
	ld de,#0x3100		; D 8", A drive motor on E track

	ld hl,#0x0100
	ld a,#0x7f
	out (0x04),a		; Side 0

	ld a,d			; set up drive
	out (0x34),a

	ld a,c

movecmd:
	out (0x30),a		; now doing a restore at lowest speed

	xor a
wait:	dec a
	jr nz,wait

wait2:	in a,(0x34)		; when restore is done we are good
	rra
	jr nc, wait2

	in a,(0x30)
	and #0x98
	jr nz, restart

	; Ok drive is up and running

	ld a,b
	out (0x32),a		; sector we want
	ld a,d
	or #0x80		; autowait
	out (0x34),a

	ld c,#0x33
	ld a,#0x9c		; read multiple 128byte, head load delay
	out (0x30),a		; load heads and read

	; autowait means we don't poll the ready bit
eoj:
	in a,(0x34)
	rra
	jr c, trackend
	ini
	jp eoj
trackend:
	;
	; FIXME: now step in until we hit track 19 (63000 bytes)
	;
	in a,(0x30)
	bit 4,a
	jr z,restart

	ld a,d
	out (0x34),a		; turn off autowait

	inc e
	ld a,#19
	cp e
	jr z, run

	ld a,#'.'		; We know that one byte per track will not
	out (1),a		; overrun!

	ld b,#0x01		; run from sector 1

	;
	; Step in a track
	;
	ld a,#0x5F		; step in with verify and update, slowest


	jr movecmd

strout:
	in a,(0)
	rla
	jr nc,strout
	ld a,(hl)
	or a
	ret z
	out (1),a
	inc hl
	jr strout

go:	.ascii 'FUZIX LOADER'
nl:	.db 13,10,0

	.ds 1

run:
	ld hl,#nl
	call strout
end:
	; and runs on into 0x100
