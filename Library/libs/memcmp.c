/* memcmp.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Z80 rewrite from UMZIX
 */
 
#include <stdlib.h>

/********************** Function memcmp ************************************/
int memcmp(void *s, void *d, size_t l) __naked
{
__asm

        push    ix
        ld      ix,#0
        add     ix,sp

        ; d = BC, s=HL, l=DE
	
        ld      l, 4(ix)
        ld      h, 5(ix)
        ld      c, 6(ix)
        ld      b, 7(ix)
        ld      e, 8(ix)
        ld      d, 9(ix)
        push    bc
        pop     iy      ; IY=d
        ld      bc,#0x0000    ; char1, char2
l_1:      
	ld      a,(hl)
        ld      b,a
        ld      a,(iy)  ; char1 != char 2 ?
        ld      c,a
        cp      b
        jr      nz,l_2
        inc     hl	; s++
        inc     iy	; d++
        dec     de	; l--
        ld      a,d
        or      e
        jp      nz,l_1	; l != 0, continue
l_2:      
	ld      a,c	; char1 - char2
        ld      e,a
	rla
	sbc	a,a
	ld	d,a
        ld      a,b
        ld      l,b
	rla
	sbc	a,a
	ld	h,a
	or	a
	sbc	hl,de

	pop ix
	ret
__endasm;
}
