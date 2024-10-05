#include "kernel-pdp11.def"

	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore
	.globl _doexec
	.globl ___udivhi3
	.globl ___umodhi3
	.globl _abort
	.globl outr0hex
	.globl outstring

	.globl _sys_cpu
	.globl _sys_cpu_feat
	.globl _sys_stubs
	.globl _set_cpu_type

/*	Probably should go to ipl6 not ipl7 to allow profiling ?

	This is slightly odd compared with most machines. Rather than
	having special move to/from control register instructions the
	status register actually lives at the very top of the unibus device
	address space - for now 077776 and the other control registers work
	in a very similar way */

___hard_di:
	mov 0177776,r0
	bis 0340,0177776
	rts pc
___hard_ei:
	mov 0177776,r0
	bic 0340,0177776
	rts pc
___hard_irqrestore:
	mov 2(sp),0177776
	rts pc

/* Mostly TODO */

_sys_cpu:
	.byte	7
_sys_cpu_feat:
	.byte	0

	.even

_sys_stubs:
	trap	$17
	rts pc
_set_cpu_type:
	rts pc


/*	FIXME: need correct code for MMU supervisor/user case */

_doexec:
#if CONFIG_PDP11_04
	mov 2(sp),r0			/* Jump address */
	mov $_udata+U_DATA__U_ISP,sp	/* user stack = kernel for simple */
	clr -(sp)
	move r0,-(sp)
	clr r0
	clr r1
	clr r2
	clr r3
	clr r4
	clr r5
	rtt
#else
	mov 2(sp),r0
	mov $_udata+U_DATA__U_ISP,r1
	mov r1,0177717		/* user mode stack pointer */
	mov $_udata+512,sp	/* rewind to the top of kernel stack */

	/* PSW is
	   [mode.2][prevmode.2][grs][revid.2][cissusp][pri.3][flags]
	   00: kernel 01: supervisor, 11 user*/

	/* Make a trap frame */
	mov $0xC000,-(sp)	/* user, was kernel */
	mov r0,-(sp)
	clr r0			/* no leaks */
	clr r1
	clr r2
	clr r3
	clr r4
	clr r5
	rtt			/* IRQ on, user space return */
#endif
	

_abort:
	rts pc

outhd:
	bicb $0x0F,r0
	add $48,r0
	cmpb 57,r0
	blos hexgood
	add $7,r0
hexgood:
	jmp outchar

out0hb:
	mov r0,-(sp)
	rorb r0
	rorb r0
	rorb r0
	rorb r0
	jsr pc,outhd
	mov (sp),r0
	jsr pc,outhd
	mov (sp)+,r0
	rts pc

outr0hex:
	jsr pc,out0hb
	swab r0
	jsr pc,out0hb
	swab r0
	rts pc

outstring:
	mov r1,-(sp)
	mov r0,r1
1:
	movb (r1)+,r0
	tstb r0
	beq 2f
	jsr pc,outchar
	jmp 1
2:
	mov (sp)+,r1
	rts pc

/*
 *	The PDP11s have a signed divide (except early machines) but not an
 *	unsigned one. For some reason the PDP-11 gcc support lib forgets to
 *	include this helper so we use our own
 *
 *	There are faster divide algorithms based upon lookup tables but they
 *	take a lot more memory.
 *
 *	WARNING: untested at this point
 */
	.text
	.even

	.globl __divmodhi3
	.globl __umodhi3
	.globl __udivhi3
/*
 *	Maybe should panic ?
 */
divide_zero:
	clr r0
	clr r1
	rts pc

divops:
	mov $1,r0
	clr r1
	tst r3
	beq divide_zero
align:
	cmp r2,r3
	bhis divide	/* if its bigger we are done */
	bmi divide	/* if the top bit is set we are done */
	tst r0
	beq divide	/* and if we ran out of bits it is done */
	asl r0
	asl r3
	br align
divide:
	tst r0		/* keep going until we've dealt with each bit we
			   set in the first pass */
	beq donediv
	cmp r3,r2
	blo wentover
	sub r3,r2	/* set the bit and account for that chunk */
	bis r0,r1
wentover:
	clc
	ror r0
	clc
	ror r3
	br divide
donediv:
	mov r2,r1	/* makes it easier to handle */
	rts pc

/*
 *	Basically a C wrapper around the helper that returns the answer
 *	in r0 and r1.
 *
 *	For the later machines I'm not clear if this or somehow mangling div
 * 	into udiv is faster.
 */
__divmodhi3:
	mov r2,-(sp)
	mov r3,-(sp)
	mov 8(sp),r2
	mov 10(sp),r3
	jsr pc,divops
	mov (sp)+,r3
	mov (sp)+,r2
	rts pc
/*
 *	And we get both forms just by looking at r0 or r1
 */
___umodhi3:
	jsr pc,__divmodhi3
	mov r1,r0
	rts pc
___udivhi3:
	jsr pc,__divmodhi3
	rts pc
