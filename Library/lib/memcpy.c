#include <string.h>

/* Z80 rewrite from UMZIX */

void *memcpy(void *dst, void *src, size_t count) __naked
{
__asm
        push    ix
        ld      ix,#0
        add     ix,sp
	
        ld e, 4(ix)
        ld d, 5(ix)
        ld l, 6(ix)
        ld h, 7(ix)
        ld c, 8(ix)
        ld b, 9(ix)
	push de
        ld      a,b
        or      c
        jr      z,_skip

        ldir

_skip:
	pop hl

	pop ix
	ret
__endasm;
}
