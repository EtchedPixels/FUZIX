;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
	.include "kernel.def"
	.include "../kernel-z80.def"

;
;	All of the fixed bank support is available as a library routine,
;	however it is a performance sensitive area. Start with
;
;	.include "../lib/z80fixedbank.s"
;
;	As well as using "usermem_std-z80.rel" in your link file for the
;	userspace access operations.
;
;
;	The when it all works you can consider following this example and
;	optimizing it hard.
;
;	Firstly we still want the core of the fixed bank support
;
	.include "../lib/z80fixedbank-core.s"

;
;	We want to provide our own optimized direct 32K bank to bank
;	copy. This is slightly crazy stuff. The fastest Z80 copy is to use
;	the stack. In the case of banked copies even more so. This can't be
;	a library routine as we have to inline the memory mapping as we have
;	no valid stack.
;
;	Copy the process memory for a fork.
;
;	A is the page base of the parent
;	C of the child
;
;	We violate all the rules of good programming for speed here.
;
;	Interrupts are off so the stack pointer is spare (Watch out for NMI
;	if your platform has an NMI to handle.
;
bankfork:
	push ix
	ld b,a
	in a,(0xF8)
	and #0xF0
	or b
	ld (cpatch0 + 1),a	; patch parent into loop
	in a,(0xF8)
	and #0xF0
	or c
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
	xor a		; 256 inner loops of 16 (total 32k)
copyloop:
	ex af,af'	; Save A as we need an A for ioports
cpatch0:
	ld a,#0		; parent bank (patched in for speed)
bankpatch1:
	out (0xF8),a
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
	out (0xF8),a
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
	pop ix
	jp map_kernel

spcache:
	.word 0
copyct:
	.byte 0

;
;	The second set of very performance sensitive routines are accesses
;	to user space. We thus provide our own modified versions of these
;	for speed
;
;	This works because user space occupies 4000-BFFF and we carefully
;	pack the kernel up so that only kernel code and vectors live in
;	that zone. In other words there is no case where we need to copy
;	between the 32K of user and the banked 32K of kernel. This in turn
;	means we know we can always map it all and ldir.
;

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_process_always
	.globl  map_kernel
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
uputget:
        ; load DE with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
	ret	; 	Z is still false

__uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_process_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_process_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	call map_process_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
	call map_process_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_process_always
	ldir
uput_out:
	call map_kernel
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_process_always
	ldir
	jr uput_out

;
__uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl	
	push de
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_process_always
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out
