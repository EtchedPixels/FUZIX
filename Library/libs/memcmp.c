#include <string.h>
/* memcmp.c
* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
* This file is part of the Linux-8086 C library and is distributed
* under the GNU Library General Public License.
*
* Z80 rewrite from UMZIX
*
* Really rewritten for Z80 by Amixgris/RT, Russia. 2014.
*/
#include <stdlib.h>
/********************** Function memcmp ************************************/
int memcmp(void *s, void *d, size_t l) __naked
{
__asm
	pop	af
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	push	af

1$:	ld	a,b
	or	c
	jr	z,2$ ; array1 = array2

	ld	a,(de)
	cp	(hl)
	inc	hl
	inc	de
	dec	bc
	jr	z,1$
	jr	nc,3$
	ld	hl,#1
	ret	
2$:	ld	h,b
	ld	l,c
	ret
3$:	ld	hl,#-1
	ret
__endasm;
}
