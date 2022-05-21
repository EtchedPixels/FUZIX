#include "../kernel-8086.def"

/*
 *	How to deal with EMM remains an interesting discussion point
 */

	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

	.globl switchin
	.globl plt_switchout
	.globl dofork


	/* udata transfer functions */

	/* Move the udata for for udata.u_ptab into the process save slot.
	   Preserve bx. We keep udata's in a buffer but that buffer can
	   live outside the kernel so udata_ds may not be in kernel_ds

	   All our callers don't mind us trashing es/di/cx

	   NOTE: Breaks if we exceeed 128 processes */
stash_udata:
	/* TODO */
	movw P_TAB__P_PAGE(%bx),%al
	addb %al,%al
	xchg %al,%ah		/* will be 00xx, want xx00 */
	pushw %es
	movw %ax,di
	movw udata_ds,%ax
	movw %ax,%es
	movw udata,%si
	movw $256,%cx
	cld
	rep movsw
	popw %es
	ret

	/* Restore the udata for process %bx, preserve %bx */
restore_udata:
	movb P_TAB__P_PAGE(%bx),%al
	addb %al,%al
	xchg %al,%ah
	pushw %ds
	movw %ax,%si
	movw udata_ds,%ax
	movw udata,%di
	movw %ax,%ds
	movw $256,%cx
	cld
	rep movsw
	popw %ds
	ret

	/* udata.u_ptab is the parent, %bx the child, preserve %bx */
fork_copy:
	/* Copy the data from the current instance into the new one
	   BEWARE: it is possible in the EMM case that the two processes
	   data will be in EMM in the same segment. In fact it's likely to
	   be the case. We need an optimised copier and EMM help for this
	   to do split maps (32K/32K) and blast two lots of 32K */
	ret

	/* Called to give up processor time. We will pop out of this in
	   potentially some other process, but we might also come back
	   directly */
plt_switchout:
	cli
	xorw %ax,%ax
	pushw %ax
	pushw %ei
	pushw %di
	pushw %bp
	movw %sp,udata+U_DATA__U_SP

	call stash_udata	

	call getproc
	push %ax
	call switchin
	jmp plt_monitor

switchin:
	/* We can trash registers here because we'll restore them
	   for the process we return as */
	mov %sp,%bp
	cli
	movw 2(%bp),%bx
	/* No MMU pointer means we are swapped out */
	movw P_TAB__P_PAGE_OFFSET+2(%bx),%si
	/* si is now our MMU pointer or 0 */
	cmpw $0,%si
	jne not_swapped
	/* TODO */
	movw P_TAB__P_PAGE_OFFSET+2(%bx),%si
not_swapped:
	cmpw udata+U_DATA__U_PTAB,%bx
	je skip_copyback	/* this can happen even with idle optimisation
				   consider two processes ready and us winning */
	call restore_udata
	cmpw udata+U_DATA__U_PTAB,%bx
	jne switchfail
skip_copyback:
	movb $P_RUNNING,P_TAB__P_STATUS_OFFSET(%bx)
	xorw %ax,%ax
	movw %ax,runticks
	movw udata+U_DATA__U_SP,%sp
	popw %bp
	popw %di
	popw %ei	
	popw %ax
	ret
switchfail:
	call  outaxhex
	mov $badswitchmsg,ax
	calloutstring		
	jmp _plt_monitor
dofork:
	pushw %bp
	movw %sp,%bp
	cli
	movw 4(%bp),%bx

	movw P_TAB__P_PID_OFFSET(%bx),%ax
	pushw %ax
	pushw %bp
	pushw %di
	pushw %ei
	movw %sp,udata+U_DATA__U_SP

	/* These two preserve bx */
	call stash_udata

	/* Stack twice as C code is entitled to mash the first one */
	pushw %bx
	pushw %bx
	call fork_copy
	popw %bx
	popw %bx

	popw %ei
	popw %di
	popw %bp
	popw %ax

	pushw %bx
	call newproc
	popw %ax

	xorw %ax,%ax		/* As child */
	movw %ax,runticks
	ret

	.data
swapstack:
	.bss 256
	.byte "_switchin: FAIL",0
	.even
