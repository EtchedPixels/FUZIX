#include "../../cpu-z80u/kernel-z80.def"
#include "kernelu.def"

#ifdef CONFIG_MAP80
	; No sneaky ways to speed this up as we can't map two different
	; banks at once
#include "../../lib/z80ufixedbank.S"
#else
#include "../../lib/z80ufixedbank-core.s"

	.common

	;	A is parent page C is child page
bankfork:
	;	We can read/write different banks
	and	0x0F	; iosolate parent page for read
	ld	b,a
	ld	a,c
	and	0xF0	; child for write
	or	b
	out	(0xFF),a	; now reading parent writing child
	ld	hl,0
	ld	d,h
	ld	e,l
	ld	bc,U_DATA_STASH+U_DATA__TOTALSIZE
	ldir
	ld	a,0x11		; kernel
	out	(0xFF),a
	ret
#endif
