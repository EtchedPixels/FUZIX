#include <string.h>
void *memccpy(void *d, const void *s, int c, size_t n) __naked
{ d,s,c,n;
__asm
	pop af	; return addr
	pop hl	; d
	pop de	; s
	exx
	pop bc	; c
	exx
	pop bc	; n
	push bc
;	exx
	push bc
;	exx
	push de
	push hl
	push af
	push hl ; save dest ptr
1$:	ld a,b
	or c
	jr z,2$
	ld a,(de)
	inc de
	exx
	cp  c
	exx
	ld (hl),a
	jr z,3$
	inc hl
	jr 1$
2$:	pop hl ; restore dest ptr
	ret
3$:	pop hl ; stack correction
	ld hl,#0x000
	ret
__endasm;
}
