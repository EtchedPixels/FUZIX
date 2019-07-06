/*
 * setjmp.c for UZIX
 * by A&L Software, 08/07/99
 *
 * FIXME: Add alt register saves
 */
#include <setjmp.h>

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_ez80)
static int _lngjmprv = 1;

int setjmp (jmp_buf env) __naked /* int jmp_buf[7] */
{
__asm
        push    ix
        ld      ix,#0
        add     ix,sp

        ; DE = env
	ld e, 4(ix)
	ld d, 5(ix)
        ; dont use stack here!

        .db    0xFD,0x7D       ; LD A,IYl      - saves IY
        ld      (de),a
        inc     de
        .db    0xFD,0x7C       ; LD A,IYh
        ld      (de),a
        inc     de
        .db    0xDD,0x7D       ; LD A,IXl      - saves IX
        ld      (de),a
        inc     de
        .db    0xDD,0x7C       ; LD A,IXh
        ld      (de),a
        inc     de
        ld      a,c             ;               - saves BC
        ld      (de),a
        inc     de
        ld      a,b
        ld      (de),a
        inc     de
        ld      a,e             ;               - saves DE
        ld      (de),a
        inc     de
        ld      a,d
        ld      (de),a
        inc     de
        ld      a,l             ;               - saves HL
        ld      (de),a
        inc     de
        ld      a,h
        ld      (de),a
        inc     de
        ld      hl,#0x0000
        add     hl,sp           ; HL = SP
        inc     hl              ; skip return address to after setjmp call
        inc     hl              ; (it is saved below)
        ld      a,l             ;               - saves SP
        ld      (de),a
        inc     de
        ld      a,h
        ld      (de),a
        inc     de
        pop     hl              ; HL = return address after setjmp call
        push    hl
        ld      a,l             ;               - saves PC
        ld      (de),a
        inc     de
        ld      a,h
        ld      (de),a
        ld      hl,#0x0000

	pop ix
	ret
__endasm;
}

void longjmp (jmp_buf env, int rv) __naked
{
__asm
        push    ix
        ld      ix,#0
        add     ix,sp

        ; BC = rv, DE = env
	
	ld e, 4(ix)
	ld d, 5(ix)
	ld c, 6(ix)
	ld b, 7(ix)

        ; dont use stack here!
        .globl  __lngjmprv

        ld      (__lngjmprv),bc
        ld      hl,#0x0012
        add     hl,de
        ld      b,(hl)
        dec     hl
        ld      c,(hl)          ; BC = return address after setjmp call
        dec     hl
        ld      a,(hl)
        dec     hl
        ld      l,(hl)
        ld      h,a             ; HL = saved SP in env
        ld      sp,hl           ;               - restores SP
        push    bc              ; put return address in stack
        ld      a,(de)
        .db    0xFD,0x6F       ; LD IYl,A      - restores IY
        inc     de
        ld      a,(de)
        .db    0xFD,0x67       ; LD IYh,A
        inc     de
        ld      a,(de)
        .db    0xDD,0x6F       ; LD IXl,A      - restores IX
        inc     de
        ld      a,(de)
        .db    0xDD,0x67       ; LD IXh,A
        inc     de
        ld      a,(de)
        ld      c,a             ;               - restores BC
        inc     de
        ld      a,(de)
        ld      b,a
        inc     de
        ld      a,(de)
        ;ld     e,a             ;               - restores DE
        inc     de
        ;ld     a,(de)
        ;ld     d,a             ; DE is the pointer, it cant be restored.
        inc     de              ; anyway, it doesnt matter for lonjmp/setjmp
        ;ld     a,(de)
        ;ld     l,a             ;               - restores HL
        inc     de
        ;ld     a,(de)          ; HL is the return value, it dont need to be
        ;ld     h,a             ; restored, because it will be overwritten.
        ld      hl,(__lngjmprv) ; return value
        ld      a,h             ; zero?

	pop ix

        or      l
        ret     nz              ; return it if not
        ld      hl,#0x0001
        ret                     ; else return 1
__endasm;
}

#endif
