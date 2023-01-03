		.module booty_mc_bootface
;
;	The VZ interface looks for a VZDOS.VZ in the root directory of a
;	FAT32 partition. We can thus build a disk with a FAT32 partition, a
;	Fuzix partition and a swap partition.
;
		.area _CODE (ABS)

		.org 0x3FE8
;
;	Nail the VZ header on ourself
;
		.ascii 'VZF1'
		.ascii 'VZ FUZIX LOADER.'
		.byte 0
		.word	0xF1
		.word	0x4000

		.org 0x4000
start:	
		di
		ld	hl,#0x4000
		ld	de,#0x8000
		ld	bc,#512
		ldir
		jp	runit
runit:
		ld	ix,#0x7000	; progress bar
		ld	c,#57		; sd data port
		ld	de,#2		; start at block number 2
		; In the right space
		; Ensure low bank is in and 0
		ld	a,#1
		out	(58),a
		; Now load it
		ld	hl,#0x4000
		ld	b,#20
		call	sdload
		ld	a,#(0x40+17)	; READ
		out	(58),a
		; Second bank flip
		ld	a,#3
		out	(58),a
		ld	hl,#0x4000
		ld	b,#20
		call	sdload
		; And back
		ld	a,#1
		out	(58),a
		; Kernel core
		ld	hl,#0x8200	; above the loader
		ld	b,#63		; load to FFFF
		call	sdload
		; And the common
		ld	hl,#0x7200
		ld	b,#7
		call	sdload
		jmp	0x9000		; into the kernel
sdload:
		; raise cs
		call	cs_raise
		; dummy read
		in	a,(c)
		call	cs_lower
		call	waitff
		a = cmd;
		out	(c),a
		xor	a
		out	(c),a
		nop
		out	(c),d	; block number
		nop
		out	(c),e
		inc	de
		ld	a,#1
		out	(c),a
		nop
		in	a,(c)
		nop
		ld	b,#32
waitok:
		in	a,(c)
		or	a
		ret	m
		djnz	waitok
fail:
		ld	(ix),#('?' & 0x3F)
		di
		halt
waitok:
		call	waitnotff
		cp	#0xFE
		jr	nz, fail
		ld	b,#0
fetch:
		ini
		dec	b
		nop
		ini
		djnz	fetch
		call	cs_raise
		ld	(ix),#0x191
		inc	ix
		; Fall into inbyte to finish
inbyte:
		ld	a,#0xff
		out	(c),a
		nop
		in	a,(c)
		ret
waitff:
		call	inbyte
		inc	a
		jr	nz,waitff
		ret
waitnotff:
		call	inbyte
		cp	#0xff
		jr	z,waitnotff
		ret
		; TODO preserve bits - check if card is r/w or not
cs_raise:
		in	a,(56)
		or	#3
		out	(56),a
		ret
cs_lower:
		in	a,(56)
		and	#0xFD
		out	(56),a
		ret
