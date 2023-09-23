;
;	The ADAM boots by loading a block (1K) from an adamnet
;	device at C800 and running it. We move to F200-F3FF
;	this leaves the adamnet support available

	.area	_BOOT(ABS)

	.org	0x0080

start:
	ld	a,#0x01
	out	(0x7F),a
	; Ensure any NMI is handled until we can kill it off
	ld	hl,#0x65ED
	ld	(0x66),hl
	ld	a,b		; save boot device
	; Ensures we are in base memory. Copy 512 bytes up below the
	; top of the OS helpers we are keeping
	ld	hl,#0xC800
	ld	de,#0x0080
	ld	bc,#0x0080
	ldir
	jp	go
go:
	ld	sp,#0xF200

begin_load:
	ld	bc,#0
	ld	de,#0x0001
	ld	hl,#0x0100
next:
	push	af
	call	0xFCF3	; read block
	jr	nz, failure
	inc	de
	inc	h
	inc	h
	inc	h
	inc	h		; Move on 1K
	ld	a,h
	cp	#0xF3
	jr	z, done
	pop	af
	jr	next
done:
	jp	0x0100
failure:
	; Should reload EOS and clean up ?
	di
	halt
