        .code
        .export _swab
                    
_swab:	pop bc
        pop hl
        pop de
        push de
        push hl
        push bc
        push bc
_swab1
        bit 7,d
        ret nz
        ld a,d
        or e
        ret z
        ld a,(hl)
        inc hl
        ld c,(hl)
        ld (hl),a
        dec hl
        ld (hl),c
        inc hl
        inc hl
        dec de
        dec de
        jr _swab1
  