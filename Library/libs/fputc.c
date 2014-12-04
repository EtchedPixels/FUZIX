/* z80 rewriting by Amixgris/RT 19-11-2014, Russia */

#include "stdio-l.h"

int fputc(int ch, FILE * fp) __naked
{ ch, fp;
__asm
	pop af
	pop de	; ch
	pop bc	; fp

	push bc
	push de
	push af
	ld (2$),de	; save ch
	
	ld  hl,#0x000c ; fp->mode
	call 3$
	ld  (1$),hl
	and a,#0x40 ; & __MODE_READING ???
	jr	z,00102$

	push bc
	push bc
	call	_fflush
	pop  af
	pop	 bc	

	ld  a,h
	or  a,l
	jr	z,00102$
;fputc.c:10: return EOF;
00152$:
	ld	hl,#0xFFFF
	ret
00102$:	
	ld  a,h
	and a,#0x03
	jr	NZ,00152$	; lower byte <> 0 
	ld	a,l
	and	a,#0x020
	jr  NZ,00152$
00105$:
	ex	de,hl
	ld  hl,#0x0008	; deflate = bufend
	call 3$
	ex	de,hl
	ld	h,b			; deflate = bufpos
	ld  l,c
	call 3$	
	or  a,a
	sbc hl,de
	jr  c,00107$
	push	bc
	push	bc
	call	_fflush
	pop	af
	pop	bc
	ld	a,h
	or	a,l
	jr	NZ,00152$	
00107$:
;fputc.c:18: *(fp->bufpos++) = ch;
	ld	h,b			; deflate = bufpos
	ld  l,c
	call 3$
	ld a,(2$)
	ld  (hl),a		; write out ch
	inc hl
	ld  d,b
	ld  e,c
	ex  de,hl
	ld  (hl),e
	inc hl
	ld  (hl),d
;fputc.c:19: fp->mode |= __MODE_WRITING;
	ld  hl,#1$
	set 7,(hl)
;fputc.c:22: if (((ch == '\n' && (v & _IOLBF)) || (v & _IONBF)) && fflush(fp))
	inc hl
	inc hl
	ld  a,#0x0a
	cp  (hl)
	jr	NZ,00112$
	inc hl
	xor a,a
	cp  (hl)
	jr	NZ,00112$	
	dec hl
	dec hl
	bit 0,(hl)
	jr	NZ,00113$
00112$:
	bit	1,(hl)
	jr	Z,00110$
00113$:
	push	bc
	push	bc
	call	_fflush
	pop	af
	pop	bc
	ld	a,h
	or	a,l
	jr	NZ,00152$ ; exit eof
00110$:	
;fputc.c:26: fp->bufwrite = fp->bufstart;	/* Nope */
	ld	hl,#0x0004
	add	hl,bc
	ex	de,hl
	
;fputc.c:25: if (v & (__MODE_IOTRAN | _IOLBF | _IONBF))
	ld	a,(1$)
	and	a, #0x03
	jr	Z,00115$
	ld  hl,#0x0006
	jr	00116$
00115$:
;fputc.c:28: fp->bufwrite = fp->bufend;	/* Yup */
	ld  hl,#0x0008
00116$:
;fputc.c:30: return (unsigned char) ch;
	add	hl, bc
	ldi
	ldi
	ld	hl,(2$)
	ld	h,#0x00
	ret
1$:	.dw	0
2$:	.dw	0

; in: hl = struct member deflate
;     de = struct base 
; out: hl = member content 
3$:	add hl,bc
	ld  a,(hl)
	inc hl
	ld  h,(hl)
	ld  l,a
	ret
__endasm;
}