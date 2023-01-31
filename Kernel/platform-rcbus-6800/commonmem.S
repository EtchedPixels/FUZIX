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

	.common

;
;	Declared as BSS so no non zero bytes here please
;
;	The stacks are a bit odd. The stack pointer points to the next byte
;	that will be written on a push so we want the top to be the last
;	byte inclusive, not exclusive as on most processors.
;
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.blkb 511
kstack_top:
	.byte 0

istack_base:
	.blkb 253
istack_top:
	.byte 0
istack_switched_sp: .word 0
