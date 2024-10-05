;
;	We are running at 0x2000 so the final work we do requires some care
;	that we don't blow ourselves up.
;

	.area	_BOOT(ABS)
	.org	0x2000

	; We get run with a valid SP but it's lurking in high space so
	; move sp for banking.

start:
	ld	(escape),sp

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
	
	jr	c, failure	; C???

	ld	(handle),a
	di

	ld 	a,#0x07		; Bank 7 top
	ld	bc,#0x7FFD
	out	(c),a
	ld	hl,#0xC000	; Wipe video
	ld	de,#0xC001
	ld	bc,#6911
	ld	(hl),#0
	ldir
	ld	a,#0x0F		; Video to wiped space
	ld	bc,#0x7FFD	; bank to 0
	out	(c),a

	; We are mapped  ROM/5/2/X
	; On the SE we are mapped ROM/5/8/X so be careful

	; Target for 0-3FFF	- will end up over ESX in a bit
	xor	a		; 16K for 0000-3FFF into bank 0
	call	load16k

	; Target for 4000-7FFF	- into bank 1
	ld	a,#0x01		; 16K for 4000-7FFF into bank 1
	call	load16k

	; Target for 8000-BFFF - load directly (8 or 2 depending)
	ld	hl,#0x8000
	call	loadblock

	; Target for C000-FFFF
	ld	a,#0x03		; 16K for C000-FFFF into bank 3
	call	load16k

	ld	a,(handle)
	rst	8
	.db	 0x9B


	;	Now we play mapping magic games

	;	No witnesses
	di

	;
	;	We stuffed our bank 5 into bank 0 while
	;	we worked with ESX-DOS. Get it back
	;	before we start running in high space
	;
	ld	bc,#0x7ffd
	ld	a,#0x09
	out	(c),a
	ld	hl,#0xC000
	ld	de,#0x4000
	ld	bc,#0x4000
	ldir

	;	And map page 0 to copy down to DIVMMC. We know the top bytes
	;	of this bank are free to drop in our trampoline
	ld	bc,#0x7ffd
	ld	a,#0x08
	out	(c),a

	;	Stack ceases to exist now

	;	Move our end code into the top of bank 0
	;	as that has free space left by the kernel map

	ld	hl,#strap
	ld	de,#0xFF00
	ld	bc,#0x0100
	ldir


	; Now the delicate bit of the operation
	jp 0xFF00

load16k:
	ld	hl,#0xC000
	ld	bc,#0x7FFD
	or	#8
	ld	(23388),a
	out	(c),a		; paging for this load
loadblock:
	ld	a,(handle)
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
	.ascii	"Upper paging fail"
	.db	13,0
badexrom:
	.ascii	"EXROM not functional"
	.db	13,0
baddock:
	.ascii	"DOCK not functional"
	.db	13,0
ohpoo:
	.ascii	'Unable to load Fuzix image'
	.db	13,0
filename:
	.asciz	'FUZIX.BIN'
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
	ld	hl,#0xC000	; Copy down bank 0 into the DIVMMC
	ld	de,#0x2000
	ld	bc,#0x2000
	ldir
	ld	a,#0x40
	out	(0xE3),a	; MAPRAM mode - page 3 is now R/O low 8K
	ld	hl,#0xE000
	ld	de,#0x2000
	ld	bc,#0x2000
	ldir

	; We loaded our pages into MMC/5/2/0 and our mapping
	; is currently MMC/5/2/4 so all we have to do is change
	; the top page and bingo... but we can't do it from here
	; because we are in a map that will change - let the kernel do it
	jp	3

failed:
	; The machine is busted by this point
	di
	halt
