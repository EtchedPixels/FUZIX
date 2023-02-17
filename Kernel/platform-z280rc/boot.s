;
;	280Boot
;
;	We get loaded from B400-BFFF. We then move to FC00 and load
;	the main OS from sectors 1-120.
;
;	Our layout is thus
;	0:	Boot block and partition table (standard Z280RC)
;	1-120:	Fuzix (or a loader)
;	248-255: Bootstrap			(us)
;
;	Life as we know it starts at 131072 and partitions shouldn't start
;	before that point
;
	.z80
	.area	BOOT	(ABS)
	.org	0xFC00

start:
	ld sp,#0xffff
	ld c,#0x08
	ld l,#0xff
;	ldctl (c),hl
	.byte 0xed,0x6e
	ld a,#0xb0
	out (0xe8),a			; refresh at 16uS
	ld l,#0
;	ldctl (c),hl
	.byte 0xed,0x6e
	out (0xa0),a			; Out of CF boot mode
	ld l,#0xfe
;	ldctl (c),hl
	.byte 0xed,0x6e
	ld a,#0xe2
	out (0x10),a			; UART config
	ld a,#0x80
	out (0x12),a			; TX on
	out (0x14),a			; RX on
	ld a,#'@'			; Debug marker
	out (0x18),a
	; Now relocate
	ld hl,#0xb400
	ld de,#0xfc00
	ld bc,#0x0200			; 512 bytes is enough for now
	ldir
	jp go
	; Now running where we expect to be (FCxx)
go:
	call outstr
	.ascii 'FILO Z280 0.01...'
	.byte 13,10,0
	ld c,#0x08
	ld l,#0
;	ldctl (c),hl
	.byte 0xED,0x6E
	ld a,#0x40
	out (0xCD),a
	ld d,#1
	ld hl,#0x0100
	ld c,#0xC0
;
;	Load sectors 1 to 120
;	(0100 -> F100)
;	which should be tons of space unless we get carried away
;
sector:
	ld a,#1			; The loader has left the upper LBA clear
	out (0xC5),a
	ld a,d
	cp #120
	jp z,boot
	out (0xC7),a		; Select sector
	ld a,#0x20
	out (0xCF),a		; Issue a READ PIO
busy:	in a,(0xCF)		; Spin for transfer
	and #0x88
	; Wait for busy to go low and DRQ to be high
	cp #0x08
	jr nz, busy
	ld b,#0
;	inirw		; Transfer sector
	.byte 0xED, 0x92
	in a,(0xCF)	; Clear status
	inc d		; And Repeat
	jr sector

boot:
	ld c,#0x08
	ld l,#0xfe
;	ldctl (c),hl
	.byte 0xED,0x6E
	call outstr
	.asciz 'Booting...'
	jp 0x0100

outstr:
	pop hl
outl:
	ld a,(hl)
	inc hl
	or a
	jr nz, printon
	jp (hl)
printon:
	ex af,af'
printw:
	in a,(0x12)
	rra
	jr nc, printw
	ex af,af'
	out (0x18),a
	nop
	nop
	nop
	nop
	jr outl

	
