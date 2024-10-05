;
;	Z80pack cpmsim loads the first (128 byte) sector from the disk
;	into memory at 0 then executes it
;	We are a bit tight on space here
;
;	Floppy loader: 
;	Our boot disc is 77 tracks of 26 x 128 byte sectors, and we put
;	the OS on tracks 58+, which means we can put a file system in the
;	usual place providing its a bit smaller than a whole disc.
;
;
;	assemble with sdasz80
;
		.abs

		.org	0

start:		jr diskload

rootdev:	.word 0			; patched by hand
swapdev:	.word 0			; ditto
		.word 0			; spare

progress:	.byte '/', '-', '\', '|'

diskload:	di
		ld sp, stack
		ld hl, 0x88
		exx
		xor a
		ld h, a
		ld b, a
		out (17), a		; sector high always 0
		out (10), a		; drive always 0
		ld a, 57		; start on track 58
		out (11), a
		exx
		ld c, 19		; number of tracks to load (56Kish)
load_tracks:	in a, (11)
		inc a			; track
		out (11), a
		xor a
		out (12), a
		ld b, 26		; sectors
load_sectors:	exx
		ld a, b
		and 3
		add a,progress
		ld l, a
		ld a, (hl)
		out (01), a
		ld a, 8
		out (01), a
		inc b
		exx

		in a, (12)
		inc a
		out (12), a		; sector
		ld a, l
		out (15), a		; dma low
		ld a, h
		out (16), a		; dma high
		xor a			; read
		out (13), a		; go
		in a, (14)		; status
		ld de, 128
		add hl, de
		djnz load_sectors	; 26 sectors = 3328 bytes
		dec c
		jr nz, load_tracks
		ld a, 0xc9		; to help debug
		ld (start), a
		ld a, 13
		out (1), a
		ld a, 10
		out (1), a
		jp 0x88

		.ds 26
stack:
		.byte 0xff
