*
*	AS68 for the CP/M 68K bootstrap tool. Loads $F000 bytes from
*	sector 2 onwards. Knows that it all fits on one logical track
*	as logical tracks are 128K
*
*	Note: CP/M's view of the disk is byteswapped. Fuzix is not so
*	we byteswap the image and run it
*
	.text
start:
	move.w #2,d0
	move.w #48,d1
	trap #2
	move.l #setdrive,d1
	move.w #32,d0
	trap #2
	move.l #settrack,d1
	move.w #32,d0
	trap #2
* 480 CPM sectors into workbuf
	move.l a6,workbuf
	move.w #479,d6
	move.w #2,d0
	move.w #49,d1
	trap #2
diskloop:
	add.l #1,sector
	move.l #setsec,d1
	move.w #32,d0
	trap #2
	move.l a6,(addr)
	add.w #128,a6
	move.l #setdma,d1
	move.w #32,d0
	trap #2
	move.l #doread,d1
	move.w #32,d0
	trap #2
	move.w #46,d1
	move.w #2,d0
	trap #2
	dbra d6,diskloop

	cmp.w #302,workbuf
	beq gogogo
	move.w #9,d0
	move.l #badload,d1
	trap #2
	rts
badload:
	dc.b "Not a bootable image$"
	.even
gogogo:
*
*	Supervisor mode
*
	move.w #2,d0
	move.w #50,d1
	trap #2
	move.w #$3e,d0
	trap #2
	move.w #$0700,sr
*
*	Copy the relocating logic high
*
	move.w #2,d0
	move.w #51,d1
	trap #2
	lea.l $bff00,a0
	lea.l loadercode,a1
	move.w #63,d0
loadloop:
	move.l (a1)+,(a0)+
	dbra d0,loadloop
*
*	And run it
*
	move.w #2,d0
	move.w #52,d1
	trap #2
	jmp $bff00

*
*	Run at BFF00 - must be relocatable code. Copies the kernel
*	image down to $400 and runs it
*
loadercode:
	move.w #2,d0
	move.w #53,d1
	trap #2
	lea.l workbuf,a0
	lea.l $400,a1
	move.w #15359,d0
setuploop:
	move.l (a0)+,(a1)+
	dbra d0,setuploop
	move.w #2,d0
	move.w #54,d1
	trap #2
	jmp $402
*
*	BIOS commands
*
	.data

setdrive:
* drive D
	dc.w 9
	dc.l $04
	dc.l 0
settrack:
* track 0
	dc.w 10
	dc.l 0
	dc.l 0
setsec:
* sector 3 (so we inc and start at 4)
	dc.w 11
sector:	dc.l 1
	dc.l 0
setdma:
* setdma
	dc.w 12
addr:	dc.l workbuf
	dc.l 0
doread:
* read
	dc.w 13
	dc.l 0
	dc.l 0
*
*	Use ds.w ds.b is broken
*
	.bss

workbuf:
	ds.w 32768
orkbuf
	dc.l 0
doread:
* rea