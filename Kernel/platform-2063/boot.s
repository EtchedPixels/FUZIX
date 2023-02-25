;
;	Bootstrap Loader. 
;
;	We hide in the 512 byte boot space in the file system. The loader
;	will load a ton of extra blocks but we just ignore that and then
;	do our own job.
;
;
	.area _BOOT(ABS)

	.org	0xFE00

; We load at C000 - relocate
start:
	di
	ld	hl,#0xC000
	ld	de,#0xFE00
	ld	bc,#0x0200
	ldir
	jp	go
go:
	ld	sp,#0x0000
	xor	a
	out	(0),a		;	Get the low page in low memory
	ld	ix,#0x0000
	ld	de,#0x7F01	;	load from the proper boot area
	jp	run		;	7F blocks 0000 to FDFF

_gpio:
	.byte	0

run:
next_block:
	push	de
	call	load_block	;	D is down count for done
	ld	a,#'.'
	call	outchar
	pop	de
	inc	e
	dec	d
	jr	nz, next_block
	call	outnl
	ld	hl,(0x0103)
	ld	de,#0x10AE
	or	a
	sbc	hl,de
	jp	z, 0x0100
	ld	a,#'X'
	call	outchar
fail:
	di
	halt

outnl:
	ld	a,#13
	call	outchar
	ld	a,#10
outchar:
	ex	af,af'
outcharw:
	xor	a
	out	(0x32),a
	in	a,(0x32)
	and	#0x04
	jr	z,outcharw
	ex	af,af'
	out	(0x30),a
	ret

load_block:
	; Load block E into IX
	; Assumes SDHC/SDXC 2GN+ as the boot ROM does likewise
	; FIXME: clean this up if the ROM changes
	push	de
	call	sd_spi_raise_cs
	call	sd_spi_receive_byte
	call	sd_spi_lower_cs
	call	waitff
	ld	l,#81		; Read block
	call	sd_spi_transmit_byte
	pop	de
	ld	l,#0
	call	sd_spi_transmit_byte
	ld	l,#0
	call	sd_spi_transmit_byte
	ld	l,#0
	call	sd_spi_transmit_byte
	ld	l,e		; block number (we look from the boot area)
	call	sd_spi_transmit_byte
	ld	l,#1
	call	sd_spi_transmit_byte
waitok:
	call	sd_spi_receive_byte
	ld	a,l
	or	a
	jp	m, waitok
waitnotff:
	call	sd_spi_receive_byte
	inc	l
	jr	z, waitnotff
	; should be FE
	inc	l
	jr	nz, fail
	ld	b,#0
data:
	call	sd_spi_receive_byte
	ld	(ix),l
	inc	ix
	call	sd_spi_receive_byte
	ld	(ix),l
	inc	ix
	djnz	data
	ret

waitff:	call	sd_spi_receive_byte
	inc	l
	ret	z
	jr	waitff

sd_spi_lower_cs:
	ld	a,(_gpio)
	and	#0xFD
	or	#0x01
	out	(0x10),a
	and	#0xFB
	ld	(_gpio),a
	out	(0x10),a
	ret
sd_spi_raise_cs:
	ld	a,(_gpio)
	and	#0xFE
	out	(0x10),a
	or	#0x05
	ld	(_gpio),a
	out	(0x10),a
	; Fall through
sd_spi_receive_byte:
	ld	a,(_gpio)
	or	#0x01		; data high
	and	#0xFD		; clock
	ld	d,a
	add	#2
	ld	e,a		; D is clock low E is high
	ld	c,#0x10
	; Entry point with registers set up
sd_rx:
	;	Clock the bits	(47 clocks a bit) or about 25K/second minus
	; 	overheads per byte - so nearer 20K which is adequate for our
	; 	needs
	;	Bit 0
	out	(c),d
	out	(c),e
	in	a,(0x00);
	rla	
	rl	l
	;	Bit 1
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 2
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 3
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 4
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 5
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 6
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 7
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	ret

;
;	Slightly less performance critical which
;	is good as it's annoying on this setup
;	due to the shared gpio
;
;	Byte to send is in L
;
sd_spi_transmit_byte:
	ld	a,(_gpio)
	and	#0xFD		; clock high
	;	Clock the bits out. Ignore reply
	;	54 clocks a bit
	;	
	;
	;	Bit 0
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 1
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub 	#2
	;	Bit 2
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 3
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 4
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 5
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 6
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a
	sub	#2
	;	Bit 7
	rra
	rl	l
	rla
	out	(0x10),a
	add	#2
	out	(0x10),a

	ret
