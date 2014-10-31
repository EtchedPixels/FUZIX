/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/********************** Function strlen ************************************/
size_t strlen(char *str) __naked
{
__asm
        pop     bc
        pop     hl
        push    hl
        push    bc
        xor     a, a
        ld      b, a
        ld      c, a
        cpir
        ld      hl, #-1
        sbc     hl, bc  ; C flag still cleared from xor above.
        ret
__endasm;
}
