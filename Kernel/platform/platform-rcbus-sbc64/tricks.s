        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

	.include "../../lib/z80fixedbank-core.s"
	.include "../../lib/z80user1.s"

;
;	This is related so we will keep it here. Copy the process memory
;	for a fork. a is the page base of the parent, c of the child
;
;	We violate all the rules of good programming for speed here.
;
;	Interrupts are off so I guess the stack pointer is spare (Watch
;	out for NMI if you ever need it!)
;
bankfork:
	ld (cpatch0 + 1),a	; patch parent into loop
	ld a,c
	ld (cpatch1 + 1),a	; patch child into loop
	;
	;	Set up ready for the copy
	;
	ld (spcache),sp
	; 32256 bytes to copy.
	; Stack pointer at the target buffer
	ld sp,#PROGBASE	; Base of memory to fork
	; 8 outer loops
	ld a,#8
	ld (copyct),a
	ld a,#252	; Count 252 * 16 byte copies
copyloop:
	ex af,af'	; Save A as we need an A for ioports
cpatch0:
	ld a,#0		; parent bank (patched in for speed)
bankpatch1:
	out (0x1f),a
	pop bc		; copy 16 bytes out of parent
	pop de
	pop hl
	exx
	pop bc
	pop de
	pop hl
	pop ix
	pop iy
	ld (sp_patch+1),sp
cpatch1:
	ld a,#0		; child bank (also patched in for speed)
bankpatch2:
	out (0x1f),a
	push iy		; and put them back into the child
	push ix
	push hl
	push de
	push bc
	exx
	push hl
	push de
	push bc
	ex af,af'	; Get counter back
	dec a
	jr z, setdone	; 252 loops ?
copy_cont:
sp_patch:
	ld sp,#0
	jp copyloop
;
;	This outer loop only runs 8 times so isn't quite so performance
;	critical
;
setdone:
	ld hl,#copyct
	dec (hl)
	jr z, copy_over
	ld a,#252
	jr copy_cont
copy_over:
	;
	;	Get the stack back
	;
	ld sp,(spcache)
	;
	;	And the correct kernel bank.
	;
	jp map_kernel

spcache:
	.word 0
copyct:
	.byte 0

