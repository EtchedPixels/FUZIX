#include "../../cpu-pdp11/kernel-pdp11.def"

	.text

	.globl _switchin
	.globl _plt_switchout
	.globl _dofork

_plt_switchout:
	bis 0340,0177776
	clr -(sp)
	mov r5,-(sp)
	mov r4,-(sp)
	mov r3,-(sp)
	mov r2,-(sp)
	mov r1,-(sp)
#ifndef CONFIG_PDP11_04
	mov 0177717,-(sp)	/* User stack ptr if not 11/04 */
#endif
	mov sp,_udata+U_DATA__U_SP
	jsr pc,_getproc
	mov r0,-(sp)
	jsr pc,_switchin
	jmp _plt_monitor

/* Simple swap based to begin with */
_switchin:
	bis 0340,0177776
	mov 2(sp),r0
	tst P_TAB__P_PAGE_OFFSET(r0)
	bne not_swapped
	mov _udata+U_DATA__U_PTAB,r1
	tst P_TAB__P_PAGE_OFFSET(r1)
	/* FIXME: irqs on for swap needed yet */
	beq its_dead_jim
	mov r0,-(sp)
	mov r1,-(sp)
	jsr pc,_swapout
	mov (sp)+,r0
	mov (sp)+,r0
its_dead_jim:
	mov sp,r1
	mov $swapstack,sp
	mov r1,-(sp)
	mov r0,-(sp)
	mov r0,-(sp)
	jsr pc,_swapper
	mov (sp)+,r0
	mov (sp)+,r0
	mov $1,P_TAB__P_PAGE_OFFSET(r0)
	mov -(sp),sp
	bis 0340,0177776
not_swapped:
	cmp _udata+U_DATA__U_PTAB,r0
	bne switchfail
	movb $P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)
	clr _runticks
	mov _udata+U_DATA__U_SP,sp
#ifndef CONFIG_PDP11_04
	mov (sp)+,0177717	/* User stack ptr if not 11/04 */
	/* TODO - user MMU state */
#endif
	mov (sp)+,r1
	mov (sp)+,r2
	mov (sp)+,r3
	mov (sp)+,r4
	mov (sp)+,r5
	mov (sp)+,r0
	rts pc
switchfail:
	jsr pc,outr0hex
	mov $badswitchmsg,r0
	jsr pc,outstring		
	jmp _plt_monitor

_dofork:
	mov 2(sp),r0
	mov r5,-(sp)
	mov r4,-(sp)
	mov r3,-(sp)
	mov r2,-(sp)
	mov r1,-(sp)
#ifndef CONFIG_PDP11_04
	mov 0177717,-(sp)	/* User stack ptr if not 11/04 */
	/* TODO - user MMU state */
#endif
	mov P_TAB__P_PID_OFFSET(r0),-(sp)
	mov sp,_udata+U_DATA__U_SP
	mov _udata+U_DATA__U_PTAB,-(sp)
	jsr pc,_swapout
	add $14,sp
	tst r0
	bne forked_up
	mov _udata,-(sp)
	mov 4(sp),-(sp)
	jsr pc,_makeproc
	add $4,sp
	clr r0
	clr _runticks
	rts pc
forked_up:
	mov $-1,r0
	rts pc

/*	.data   For now: no split I/D yet */
badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13,10,0
	.even
	.= .+256
swapstack:
