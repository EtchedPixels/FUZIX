;
;	Common space for stacks and user data
;
        ; exported symbols
        .globl ub
        .globl udata
        .globl kstack_top
        .globl istack_top
        .globl istack_switched_sp

	.section .data

;
;	The stacks are a bit odd. The stack pointer points to the next byte
;	that will be written on a push so we want the top to be the last
;	byte inclusive, not exclusive as on most processors.
;
;	.ds is in words for this assembler ????
;
ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
udata:
kstack_base:
	.ds 255,0
	.byte 0
kstack_top:
	.byte 0

istack_base:
	.ds 126,0
	.byte 0
istack_top:
	.byte 0
istack_switched_sp: .word 0

;
;	We have a tiny writable common area in the internal ram
;
	.section .commondata

	.globl shared_sp
	.globl shared_syscall
	.globl shared_map
	.globl shared_mlatch
	.globl shared_argn
	.globl shared_argn1
	.globl shared_argn2
	.globl shared_argn3
	.globl shared_retval
	.globl shared_error
	.globl shared_signal
	.globl shared_sigvec
	.globl shared_preempt
	.globl int_stack
	.globl syscall_stack
	.globl sci_stack

shared_sp:
	.word	0
shared_syscall:
	.byte	0
shared_map:
	.byte	0
shared_mlatch:
	.byte	0
shared_argn:
	.word	0
shared_argn1:
	.word	0
shared_argn2:
	.word	0
shared_argn3:
	.word	0
shared_retval:
	.word	0
shared_error:
	.word	0
shared_signal:
	.byte	0
shared_sigvec:
	.word	0
shared_preempt:
	.byte	0
	.ds	16		; words
int_stack:
	.byte	0

	.ds	16

syscall_stack:
	.byte	0

	.ds	16		; Stack for serial interrupt
sci_stack:
	.byte	0
