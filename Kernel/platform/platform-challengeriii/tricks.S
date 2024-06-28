;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;
;	All of the fixed bank support is available as a library routine,
;	however it is a performance sensitive area. Start with
;
; #include "../../lib/z80ufixedbank.S"
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
#include "../../lib/z80ufixedbank-core.s"

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
	push iy
	ld (cpatch0 + 1),a	; patch parent into loop
	ld a,c
	ld (cpatch1 + 1),a	; patch child into loop
	;
	;	Set up ready for the copy
	;
	ld (spcache),sp
	; 48640 bytes to copy.
	; Stack pointer at the target buffer
	ld sp, PROGBASE	; Base of memory to fork
	; 16 outer loops of 190 bytes (16 x 16 x 190)
	ld a, 16
	ld (copyct),a
	ld a,190	; 190 inner loops of 16 (total 32k)
copyloop:
	ex af,af'	; Save A as we need an A for ioports
cpatch0:
	ld a, 0		; parent bank (patched in for speed)
	ld (0xF700),a
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
	ld a, 0		; child bank (also patched in for speed)
	ld (0xF700),a
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
	ld sp, 0
	jp copyloop
;
;	This outer loop only runs 16 times so isn't quite so performance
;	critical
;
setdone:
	ld hl, copyct
	dec (hl)	
	jr z, copy_over
	ld a, 252
	jr copy_cont
copy_over:
	;
	;	Get the stack back
	;
	ld sp,(spcache)
	pop iy
	pop ix
	;
	;	And the correct kernel bank.
	;
	jp map_kernel

spcache:
	.word 0
copyct:
	.byte 0

