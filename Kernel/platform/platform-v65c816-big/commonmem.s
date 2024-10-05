;
;	We keep our common area right down low, with the ZP and stack
;
;
        ; exported symbols
        .export _ub
        .export _udata
        .export kstack_top
	.export kstack_base
	.export kstackc_top
        .export istack_top
	.export istackc_top
        .export istack_switched_sp

        .segment "COMMONDATA"
	.include "zeropage.inc"

	.p816
	.a8
	.i8

;
;	The udata for 65C816 is a bit different to 6502 as we have both a C
;	stack and a small CPU stack in the banking
;
;	Our current layout is
;	[udata]				~256 bytes (279 with L2)
;	[C stack]			256  (or thereabouts)
;
;	There is a separate IRQ DP, stack and C stack.
;
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstackc_base:
	.res 512,0
kstackc_top:
	; and our saved area follows this by a copy of kstack

;
;	We have a single istack so we can stuff that anywhere we like
;
	.bss

istackc_base:
	.res 254,0			; overkill - tune me
istackc_top:
istack_switched_sp:
	.word 0


;
;	Stacks go in bank 0 0100-02FF
;
	.segment "STACK"

kstack_base:
	.res 256,0
kstack_top:

istack_base:
	.res 64,0			; should be tons
istack_top:
