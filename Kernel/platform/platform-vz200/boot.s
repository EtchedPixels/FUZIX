;
;	Alternate bootrom image for the SD interface (can be disk loaded)
;
;	TODO:
;	Set speed once we know type
;	Pass speed and type to boot block
;	Dump original ROM to find correct delays
;
		.area BOOT	(ABS)


		.org 0x3FE8
;
;	Nail the VZ header on ourself
;
		.ascii 'VZF0'
		.ascii 'VZ FUZIX LOADER.'
		.byte 0
		.byte	0xF1
		.word	0x4000

		.org 0x4000

cardtype .equ	0x78FF
buf	.equ	0x78FB
spos	.equ	0x78F9

CT_MMC		.equ	0x01
CT_SD2		.equ	0x02
CT_SDBLK	.equ	0x03
CT_SD1		.equ	0x04

	.byte 0xAA
	.byte 0x55
	.byte 0xE7
	.byte 0x18
boot:
	di
	ld	hl,#0
	add	hl,sp
	ld	sp,#spos
	push	hl

	call	wipe

	call	pstring
	.ascii	"FUZIX BOOTSTRAP LOADER"
	.byte	13,0

	call	sd_init
	ld	(cardtype),a
	cp	#0xFF
	jr	nz, card_ok
	call	pstring
	.ascii	"SD CARD DETECT FAILED"
	.byte	13,0
basic:
	call	pstring
	.ascii	"HIT A KEY FOR BASIC"
	.byte	13,0
	pop	hl
	ld	sp,hl
kwait:
	ld	a,(0x6800)		;	keyboard all rows
	and	#0x7f
	cp	#0x7f
	jr	z, kwait
	ret
wipe:
	ld	hl,#0x7000
	ld	(spos),hl
	ld	de,#0x7001
	ld	bc,#0x1FF
	ld	(hl),#0x60
	ldir
	ret
card_ok:
	call	pstring
	.ascii	"SD CARD INITIALIZED"
	.byte	13,0

loader:
	call	csraise
	ld	hl,#cmd17
	call	sendcmd
	jp	nz, sdfail2
waitdata:
	call	sendff
	cp	#0xFE
	jr	nz,waitdata
	ld	hl,#0x8800
	ld	b,#0
dataloop:
	call	sendff
	ld	(hl),a
	inc	hl
	call	sendff
	ld	(hl),a
	inc	hl
	djnz	dataloop
	call	csraise
	ld	a,(0x8800)
	cp	#0x23
	jr	nz, noboot
	ld	a,(0x8801)
	or	a
	jr	nz, noboot
	call	pstring
	.ascii  'BOOTING'
	.byte	13,0
	;	We loaded 512 bytes at 8800-89FF
	ld	a,(cardtype)	; card type
	jp	0x8802		; after the signature

noboot:
	call	wipe
	call	pstring
	.ascii	'SD NOT BOOTABLE'

	.byte	13,0
	jp	basic

	;	SD card init (based on my tiny 68HC11 loader)

old_card:	; SDSC or MMC
	ld	hl,#acmd41_0		; Send ACMD41
	call	sendacmd
	cp	#2
	jr	nc, mmc
wait41_0:
	ld	hl,#acmd41_0
	call	sendacmd
	jr	nz, wait41_0
	ld	a,#CT_SD1
	jr	secsize
mmc:
	ld	hl,#cmd1
	call	sendcmd
	jr	nz, mmc
	ld	a,#CT_MMC
secsize:
	push	af
	ld	hl,#cmd16
	call	sendcmd
	jr	nz,sdfail2
	pop	af
	ret
sd_init:
	ld	a,#0x00
	out	(56),a		; CS high and slow
	ld	b,#32
csloop:	call	sendff
	djnz	csloop
	ld	hl,#cmd0
	call	sendcmd
	dec	a
	jr	nz, sdfail
	ld	hl,#cmd8
	call	sendcmd
	dec	a		; Should be 1 for SDHC
	jr	nz, old_card
	;	Ok it's not an early SD or MMC
	call	get4
	ld	hl,(buf+2)
	ld	de,#0xAA01
	or	a
	sbc	hl,de
	jr	nz, sdfail
wait41:
	ld	hl,#acmd41
	call	sendacmd
	jr	nz,wait41
	ld	hl,#cmd58
	call	sendcmd
	jr	nz,sdfail
	call	get4
	ld	a,(buf)
	and	#0x40
	ld	a,#CT_SDBLK
	ret	nz		; block oriented card
	ld	a,#CT_SD2
	ret
sdfail2:
	pop	af
sdfail:
	ld	a,#0xFF
	ret

sendacmd:
	push	hl
	ld	hl,#cmd55
	call	sendcmd
	pop	hl
sendcmd:
	call	csraise
	call	cslower
	ld	a,(hl)
	cp	#0x40		; CMD0
	jr	z, nowaitff
waitff:
	call	sendff
	inc	a
	jr	nz, waitff
nowaitff:
	ld	bc,#(0x0600 | 57)
sendlp:
	outi
	nop
	nop
	jr	nz, sendlp
	call	sendff
waitret:
	call	sendff
	or	a
	jp	m,waitret
	ret
get4:
	ld	b,#4
	ld	hl,#buf
get4l:
	call	sendff
	ld	(hl),a
	inc	hl
	djnz	get4l
	ret

cslower:
	ld	a,#3
	out	(56),a
	ret
csraise:
	ld	a,#1
	out	(56),a
sendff:	
	ld	a,#0xff
send:
	out	(57),a
	nop
	nop
	nop
	nop
	in	a,(57)
	or	a
	ret

; Some minimal text output logic

pstring:
	exx
	pop	hl
ploop:
	ld	a,(hl)
	inc	hl
	or	a
	jr	z,strdone
	call	pchar
	jr	ploop
strdone:
	push	hl
	exx
	ret

pchar:
	push	hl
	ld	hl,(spos)
	cp	#13
	jr	z,newline
	and	#0x3F
	add	#0x40
	ld	(hl),a
	inc	hl
	ld	(spos),hl
	pop	hl
	ret
newline:
	ld	a,l
	add	#31
	jr	nc, nobankh
	inc	h
nobankh:
	and	#0xE0
	ld	l,a
	ld	(spos),hl
	pop	hl
	ret

;
;	Commands
;
cmd0:
	.byte 0x40,0,0,0,0,0x95
cmd1:
	.byte 0x41,0,0,0,0,0x01
cmd8:
	.byte 0x48,0,0,0x01,0xAA,0x87
cmd16:
	.byte 0x50,0,0,2,0,0x01
cmd17:
	.byte 0x51,0,0,0,0,0x01
cmd55:	
	.byte 0x77,0,0,0,0,0x01
cmd58:
	.byte 0x7A,0,0,0,0,0x01
acmd41_0:
	.byte 0x69,0,0,0,0,0x01
acmd41:
	.byte 0x69,0x40,0,0,0,0x01
