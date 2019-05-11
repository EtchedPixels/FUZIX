;
;	ROM patch for Tom's SBC
;
	.area CODE(ABS)

.org 0x3FE2

;	HL = SOURCE
;	IX = DESTINATION
;	BC = COUNT
;	D = destination bank
;	E = source bank
;	
;
;
ldir_far:
	    push bc
	    ld c,#0x3e
	    exx
	    pop bc			; get BC into alt bank
far_ldir_1:
	    exx
	    out (c),e
	    ld a,(hl)
	    inc hl
	    out (c),d
	    ld (ix),a
	    inc ix
	    exx
	    dec bc
	    ld a,b
	    or c
	    jr nz, far_ldir_1
	    xor a
	    out (0x3e),a
	    ret

	    jp ldir_far


