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
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.blkb 512
kstack_top:

istack_base:
	.blkb 254
istack_top:
istack_switched_sp: .word 0
