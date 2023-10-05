	.include "../../cpu-z80/kernel-z80.def"

	.include "../../lib/z80fixedbank-core.s"

	.area _COMMONMEM

	;	A is parent page C is child page
bankfork:
	;	We can read/write different banks
	and	#0x0F	; iosolate parent page for read
	ld	b,a
	ld	a,c
	and	#0xF0	; child for write
	or	b
	out	(0xFF),a	; now reading parent writing child
	ld	hl,#0
	ld	d,h
	ld	e,l
	ld	bc,#U_DATA_STASH+U_DATA__TOTALSIZE
	ldir
	ld	a,#0x11		; kernel
	out	(0xFF),a
	ret
