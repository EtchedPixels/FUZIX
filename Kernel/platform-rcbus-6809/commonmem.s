;
        .module commonmem

        ; exported symbols
        .globl _ub
        .globl _udata
        .globl kstack_top
        .globl istack_top
        .globl istack_switched_sp

        .area .udata,bss

_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	zmb 512
kstack_top:

        ; next 256 bytes: 254 byte interrupt stack, then 2 byte saved stack pointer
istack_base:
	zmb 254
istack_top:
istack_switched_sp: .dw 0
