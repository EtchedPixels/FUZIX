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
	.export kstack_cpu
	.export kstack_cpu_top

	.common

;
;	In 6502 land these are the C stacks but also CPU stacks.
;
;	Commondata exists per process in the switched space
;
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.ds 512
kstack_top:

istack_base:
	.ds 254
istack_top:
istack_switched_sp: .word 0

kstack_cpu:
	.ds 255
kstack_cpu_top:
	.byte 0