;
        .module commonmem

        ; exported symbols
        .globl _ub
        .globl _udata
        .globl kstack_top
        .globl istack_top
        .globl istack_switched_sp

        .area .udata

_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	zmb 512
kstack_top:

	.area .istack

;
;	Be careful this is a bit tight
;
istack_base:
	zmb 224
istack_top:
istack_switched_sp: .dw 0
