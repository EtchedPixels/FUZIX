#include "../kernel-8086.def"

	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

	.globl switchin
	.globl switchout
	.globl dofork

switchout:
	cli
	call chksigs
	pushw %ax		/* Figure out what we actually need to save */
	pushw %bx
	pushw %cx
	pushw %dx
	pushw %es
	pushw %di
	pushw %bp
	movw %sp,udata+U_DATA__U_SP
	call getproc
	push %ax
	call switchin
	jmp trap_monitor

switchin:
	push %bp
	mov %sp,%bp
	cli
	movw 4(%bp),%si
	cmpb $0,P_TAB__P_PAGE_OFFSET(%si)
	/*
	jne not_swapped
	 TODO */
	ret
switchfail:
/*	call  outaxhex
	mov $badswitchmsg,ax
	calloutstring		
	jmp _trap_monitor */

dofork:
	/* TODO */
	movw $-1,%ax
	ret

	.data
swapstack:
	.bss 256
