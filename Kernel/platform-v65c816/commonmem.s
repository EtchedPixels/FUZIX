;
;	We keep our common area right down low, with the ZP and stack
;
;
        ; exported symbols
        .export _ub
        .export _udata
        .export kstack_top
        .export istack_top
        .export istack_switched_sp
	.export CTemp

        .segment "COMMONDATA"
	.include "zeropage.inc"

;
;	In 6502 land these are the C stacks, we will need to handle the
;	hardware stack separately, and also to save sp,sp+1 etc on irqs
;
;	Declared as BSS so no non zero bytes here please
;
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.res 512,0
kstack_top:
FIXME: C stack of 512 - udata, 65C816 stack follows

;
;	We have a single istack so we can stuff that anywhere we like
;
	.bss

istack_base:
	.res 254,0
istack_top:
FIXME: interrupt CPU stack (64 ?)
istack_switched_sp: .word 0
