;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

;
;	All of the fixed bank support is available as a library routine,
;	however it is a performance sensitive area. Start with
;
;	.include "../lib/z80fixedbank.s"
;
;	As well as using "usermem_std-z80.rel" in your link file for the
;	userspace access operations.
;
;	We can use the fast Z80 copiers in this case
;
	.include "../../lib/z80user1.s"
;
;	The when it all works you can consider following this example and
;	optimizing it hard.
;
;	Firstly we still want the core of the fixed bank support
;
	.include "../../lib/z80fixedbank-core.s"

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
	xor a		; 256 inner loops of 16 (total 32k)
copyloop:
	ex af,af'	; Save A as we need an A for ioports
cpatch0:
	ld a,#0		; parent bank (patched in for speed)
bankpatch1:
	out (0x30),a
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
	out (0x30),a
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

