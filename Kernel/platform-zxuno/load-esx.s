;
;	We are running at 0x2000 so the final work we do requires some care
;	that we don't blow ourselves up.
;

	.area _BOOT(ABS)
	.org 0x2000

	; We get run with a valid SP but it's lurking in high space so
	; move sp for banking.

start:
	ld (escape),sp
	;
	;	48K and 128K ROM differ at this address
	;
	ld	a, (2899)
	cp	#126
	jr	z, ok_plus2a
	cp	#159
	jr	z, ok_128k
	;
	;	Double check: 48K ROM
	;
	di
	ld	bc, #0x7ffd
	ld	a, (23388)
	and	#0xEF
	out	(c), a

	ld	a,(2899)
	cp	#165
	jr	nz, ok_usr0

	ld	hl, #wrongbox
	jp	fail

ok_usr0:
	ld	a,(23388)
	out	(c),a
	ei
ok_plus2a:
ok_128k:
	ld	bc,#0xfc3b
	ld	a,#0xff
	out	(c),a
	inc	b
uno_check:
	in	a,(c)
	or	a
	jr	z, is_uno
	jp	m, not_uno
	cp	#32
	jr	c, not_uno
	rst	0x10
	dec	e
	jr	nz, uno_check
not_uno:
	ld	hl,#notuno
	jp	fail

is_uno:
	ld	a,#13
	rst	0x10

	ld	bc,#0xfc3b

	ld	a,#0x0E
	out	(c),a
	inc	b
	ld	a,#0x40		; sd on, timex mmu on, 1FFD and 7FFD on
				; AY chips on
	out	(c),a
	dec	b

	ld	a,#0x0F
	out	(c),a
	inc	b
	in	a,(c)
	and	#0xF0
	out	(c),a

	dec	b
	xor	a
	out	(c),a
	inc	b
	in	a,(c)
	and	#0xEF
	or	#0x46
	out	(c),a

	dec	b
	ld	a,#0x0B
	out	(c),a
	inc	b
	in	a,(c)
	and	#0x23
	or	#0x80
	out	(c),a

	di
	ld	bc,#0x7FFD
	xor	a
	out	(c),a
	ld	hl,#0xC000
	ld	e,(hl)
	ld	(hl),#0
	inc	a
	out	(c),a
	ld	d,(hl)
	ld	(hl),#1
	dec	a
	out	(c),a
	cp	(hl)
	jr	z, paging_ok
	ld	(hl),e
	inc	a
	out	(c),a
	ld	(hl),d
	ld	hl,#paging
	jp	fail

paging_ok:
	ld	(hl),e
	inc	a
	out	(c),a
	ld	(hl),d

	ld	sp,#0x8000	; bank 5 which isn't going to get stomped yet
	xor	a
	rst	8
	.db	0x89
	ld	(drive),a

	ld	hl, #filename
	ld	b, #0x01
	ld	a,(drive)

	rst	8
	.db	0x9A
	
	jr	c, failure		; C???

	ld	(handle),a
	di

	; We are mapped  ROM/5/2/X
	; Load our pages into 4,7,2,3 which will becoime
	; DIVMMC 16K, 5, 2, 3

	; Target for 0-3FFF	- will end up over ESX in a bit
	ld a,#0x04		; 16K for 0000-3FFF into bank 4
	call load16k

	; Target for 4000-7FFF	- bounce this into place at the end
	ld a,#0x07		; 16K for 4000-7FFF into bank 7
	call load16k

	; Target for 8000-BFFF
	ld a,#0x02		; 16K for 8000-BFFF into bank 2
	call load16k

	; Target for C000-FFFF
	ld a,#0x03		; 16K for C000-FFFF into bank 3
	call load16k

	ld a,(handle)
	rst 8
	.db 0x9B


	;	Now we play ZX Uno mapping magic games

	;	No witnesses
	di

	;
	;	We stuffed our bank 5 into bank 7 while
	;	we worked with ESX-DOS. Get it back
	;	before we start running in high space
	;
	ld	bc,#0x7ffd
	ld	a,#0x07
	out	(c),a
	ld	hl,#0xC000
	ld	de,#0x4000
	ld	bc,#0x4000
	ldir

	;	And map page 4 top
	ld	bc,#0x7ffd
	ld	a,#0x04
	out	(c),a

	;	Stack ceases to exist now

	;	Move our end code into the top of bank 4
	;	as that has free space

	ld	hl,#strap
	ld	de,#0xFF00
	ld	bc,#0x0100
	ldir


	; Now the delicate bit of the operation
	jp 0xFF00

load16k:
	ld	bc,#0x7FFD
	ld	(23388),a
	out	(c),a		; paging for this load
	ld	a,(handle)
	ld	hl,#0xC000
	ld	bc,#0x4000
	rst	8
	.byte	0x9D
	ret	nc
failure:
	ld	hl,#ohpoo
fail:
	ld	a,(hl)
	inc	hl
	or	a
	jr	z, faildone
	rst	0x10
	jr	fail
	; throw ourselves under the bus
faildone:
	ld	a,(handle)
	or	a
	jr	z, noclose
	rst	8
	.byte	0x9B
noclose:
	ld	sp,(escape)
	ret

wrongbox:
	.ascii "128K system required"
	.db	13,0
notuno:
	.ascii "Wrong kernel. ZX-Uno required"
	.db	13,0
paging:
	.ascii "Upper paging fail"
	.db	13,0
ohpoo:
	.ascii	'Unable to load Fuzix image'
	.db	13,0
filename:
	.asciz 'FUZIX.BIN'
handle:
	.db 0
drive:
	.db 0
escape:
	.dw 0

;
;	Code run from 0xFF00 (must be relocateable)
;	interrupts off, stack not safe to use

strap:
	ld	sp,#0xFFFF
	;
	; The DIVMMC likes to map itself in on various addresses below
	; 0x4000 by magic. This is annoying so we have to work around the
	; resulting mess. (The Uno has a bit to control this but the
	; designers for some reason put it behind the boot time lock bit)

	; The Uno designers also unfortunately broke their DIVMMC
	; implementation by ignoring the 48K ROM check in the proper DIVMMC
	;
	; Zesarux the emulator is even more spectacularly broken and adds
	; some extra brokenness to the whole sad catastrophe

	ld	a,#0x80
	out	(0xE3),a	; ESX EPROM forced mapped low
	ld	a,#0xC9		; RET
	ld	(0x3D00),a	; autopage range
	call	0x3D00		; force page
	ld	a,#0x83		; ESX EPROM and page 3
	out	(0xE3),a
	ld	hl,#0xC000
	ld	de,#0x2000
	ld	bc,#0x2000
	ldir
	ld	a,#0x40
	out	(0xE3),a	; MAPRAM mode - page 3 is now R/O low 8K
	ld	hl,#0xE000
	ld	de,#0x2000
	ld	bc,#0x2000
	ldir

	; We loaded our pages into MMC/5/2/3 and our mapping
	; is currently MMC/5/2/4 so all we have to do is change
	; into the top age and bingo... but we can't do it from here
	; because we are in a map that will change - let the kernel do it
	jp	3

failed:
	; The machine is busted by this point
	di
	halt
