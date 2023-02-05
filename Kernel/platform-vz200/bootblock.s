;
;	Boot block - loader at 8800 by the SD ROM (or by a vz loader)
;
		.module bootblock
;
;	The VZ interface looks for a VZDOS.VZ in the root directory of a
;	FAT32 partition. We can thus build a disk with a FAT32 partition, a
;	Fuzix partition and a swap partition.
;
;	TODO: 512 v 1 byte sizing
;
		.area BOOT (ABS)

		.org 0x87E8
;
;	Nail the VZ header on ourself
;
		.ascii 'VZF0'
		.ascii 'VZ FUZIX LOADER.'
		.byte 0
		.byte	0xF1
		.word	0x8800

		.org 0x8800
		; On entry DI, card type in A
		; ROM paged in, vectors at 4006

CT_MMC		.equ	0x01
CT_SD2		.equ	0x02
CT_SDBLK	.equ	0x03
CT_SD1		.equ	0x04

start:	
	inc	hl		; 23 00 Z300 header
	nop
	ld	sp,#0x89FF
	;	Check if the system has banked video
	ld	iy,#0x7180	; Screen for progress bar
	ld	(iy),#0xFF
	ld	a,#1
	out	(0x20),a
	ld	(iy),#0x60
	xor	a
	out	(0x20),a
	ld	a,(iy)
	cp	#0x60
	push	af
	ld	(0x89FF),a	; above stack
	ld	de,#1
	ld	hl,#0x4000
	ld	b,#20		; 4000-67FF worth
	ld	a,#1
	out	(55),a		; Low RAM bank 0
	call	load_sectors
	ld	b,#20
	ld	hl,#0x4000
	ld	a,#3
	out	(55),a
	call	load_sectors	; Low RAM bank 1
	; Bank pages now loaded
	; Load the main kernel block
	ld	a,#1
	out 	(55),a
	ld	hl,#0x9000
	ld	b,#56
	call	load_sectors
	; Load the extra common bits
	ld	hl,#0x7200	; 7200-87FF
	ld	b,#11
	call	load_sectors
	pop	af
	jp	z, 0x9000
	ld	iy,#0		; So we don't scribble on the loading data
	ld	hl,#0x7000
	ld	b,#4		; load 2K into video bank 3
	ld	a,#3
	out	(0x20),a
	call	load_sectors
	xor	a
	out	(0x20),a
	jp 	0x9000

load_sectors:
	push	bc
	call	load_sector
	pop	bc
	inc	de
	djnz	load_sectors
	ret

load_sector:
	ld	ix,#cmd17
	push	de
	push	hl
	ld	a,(0x89FF)
	cp	#CT_SDBLK
	jr	nz, byte_offset
	;	LBA block offset
	ld	1(ix),#0
	ld	2(ix),#0
	ld	3(ix),d
	ld	4(ix),e
	jr	do_load
byte_offset:
	sla	e		; DE is in 512s. Get it in 256s
	rl	d
	ld	1(ix),#0
	ld	2(ix),d
	ld	3(ix),e
	ld	4(ix),#0
do_load:
	ld	(iy),#0xC0
	inc	iy
	ld	hl,#cmd17
	call	sendcmd
waitdata:
	call	sendff
	cp	#0xFE
	jr	nz,waitdata
	pop	hl
	ld	b,#0
dataloop:
	call	sendff
	ld	(hl),a
	inc	hl
	call	sendff
	ld	(hl),a
	inc	hl
	djnz	dataloop
	pop	de
	jr	csraise
	
;	SD glue
;
sendcmd:
	; No need to handle ACMD here
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

cmd17:
	.byte 0x51,0,0,0,0,0x01
