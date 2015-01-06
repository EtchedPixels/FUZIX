/* memset.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Z80 rewrite from UMZIX
 */

#include <stdlib.h>

/********************** Function memset ************************************/
void *memset(void *str, int c, size_t l) __naked
{
__asm
        push    ix
        ld      ix,#0
        add     ix,sp

	ld l, 4(ix)
	ld h, 5(ix)
	ld d, 6(ix)
	ld c, 8(ix)
	ld b, 9(ix)
	ld a,b
	or c	; l=0? so return
	jr z,_retw
	ld a,d
	ld (hl),a	; fill first byte
	ld d,h
	ld e,l
	inc de	; DE=str+1
	dec bc
	ld a,b	; l=1? so return
	or c
	jr z,_retw
	push hl
	ldir
	pop hl
	
_retw:
	pop ix
	ret
__endasm;
}
