;
;	AQ+ Loader
;

	.abs

	.org 0x38E1

	; Header and BASIC stub
	.byte   0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
	.byte	0xFF,0xFF,0xFF,0xFF,0x00
	.ascii	"AQPLUS"
	.byte	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
	.byte	0xFF,0xFF,0xFF,0xFF,0x00
	.byte	0x0E,0x39,0x0A,0x00,0xDA
	.ascii	"14608:"
	.byte	0x80,0x00,0x00,0x00
	jp      start

start:
	di
	;	Turbo on
	ld	a,0x04
	out	(0xFB),a
	;	80 column alt border mode
	ld	a,0x61
	out	(0xE0),a

	;	Clear video memory
	ld	hl,0x3000
	ld	de,0x3001
	ld	bc,0x07FF
	ld	(hl),0
	ldir
	;	Clear colour RAM
	ld	hl,0x3300
	ld	de,0x3301
	ld	bc,0x7FF
	ld	(hl),0x8F
	ldir

	ld	ix,0x8000		;	video progress

	;	Now sort the mappings out

	ld	a,0x3C			;	3C at the bottom
	out	(0xF0),a

	ld	sp,0x0100

	;	Put ourselves low

	ld	hl,runhigh
	ld	de,0x0000
	ld	bc,0x0100

	ldir

	ld	hl,go
	call	puts

	;	Disk I/O setup

	ld	hl, opencmd		; open the file
	ld	b,12
	call	cmd
	jp	m, failed
	;	A is the handle

	ld	hl,readcmd + 1		; begin the read
	ld	(hl),a
	dec	hl
	ld	b,4
	call	cmd
	jp	m, failed
	call	data			; get the length
	ld	e,a
	call	data
	ld	d,a

	;	DE bytes are pending for the I/O and will be read
	;	after the memory is remapped
	rst	0

failed:
	ld	hl,bad
	call	puts
	di		; TODO
	halt

puts:
	ld	a,21
	out	(0xF3),a
putl:	ld	a,(hl)
	or	a
	jr	z,putsdone
	ld	(ix + 0),a
	inc	hl
	inc	ix
	jr	putl
putsdone:
	ld	a,0x3F
	out	(0xF3),a		; use the page we finally want
	ret
	

cmd:
	ld	c,0xF5
	in	a,(0xF4)
	rra
	jr	nc,flushed
	in	a,(0xF5)
	jr	cmd
flushed:
	ld	a,0x80
	out	(0xF4),a
wait_tx:
	in	a,(0xF5)
	and	2
	jr	nz, wait_tx
	outi
	djnz	wait_tx
data:
	in	a,(0xF4)
	rra
	jr	nc,data
	in	a,(0xF5)
	or	a		; set flags for caller
	ret

opencmd:
	.byte	0x10		; open
	.byte	0x00		; read only
	.ascii	'FUZIX.BIN'	; name
	.byte	0x00		; end marker
readcmd:
	.byte	0x12		; read
	.byte	0x00		; handle (filled in as we run)
	.word	0xFD00		; up to 64K minus our loader and stack
go:
	.ascii	'Loading FUZIX'
	.byte	0
bad:
	.ascii	'.. failed'
	.byte	0

	; Runs from 0
runhigh:
	ld	a,0x3D
	out	(0xF1),a
	inc	a
	out	(0xF2),a
	inc	a
	out	(0xF3),a
	;	Map is now 3C 3D 3E 3F as we want

	;	DE bytes are waiting for loading from the IOP

	ld	bc,0xF5
	ld	hl,0x0100
	inc	d
	inc	e
	jr	nextbyte

waitbyte:
	in	a,(0xF4)
	rra
	jr	nc,waitbyte
	ini
nextbyte:
	dec	e
	jr	nz, waitbyte
	ld	a,21
	out	(0xF1),a		; map in video
	ld	(ix + 0),'.'
	inc	ix
	ld	a,0x3D
	out	(0xF1),a		; put the map back
	dec	d
	jr	nz, waitbyte

	;	DE bytes loaded so go go go

	jp	0x0100
