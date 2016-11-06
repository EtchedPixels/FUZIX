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
	

___udivhi3:
___umodhi3:
_abort:
	rts pc

outr0hex:
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
