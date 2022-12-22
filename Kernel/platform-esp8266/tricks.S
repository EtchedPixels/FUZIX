#include "kernel-esp8266.def"

#define ISTACK_SIZE 516		/* 512 for the stack 4 for the previous stack save */
#define SWAPSTACK_SIZE 1536

.global istack_end
.global swapstack_end

.data
.section .bss
swapstack:
    .fill SWAPSTACK_SIZE
swapstack_end:

istack:
    .fill ISTACK_SIZE
istack_end:

.global plt_switchout
.global switchin
.text
.align 4
plt_switchout:
	/* Save the callee-saved registers onto the stack in the same order
	 * that dofork does it, so switchin will restore it correctly. */

	addi sp, sp, -8*4
	s32i.n a0, sp, 7*4
	s32i.n a12, sp, 0*4
	s32i.n a13, sp, 1*4
	s32i.n a14, sp, 2*4
	s32i.n a15, sp, 3*4
	rsil a3,15
	rsr.sar a3
	s32i.n a3, sp, 4*4

	/* Save stack pointer */

	movi a3, udata
	s32i.n sp, a3, U_DATA__U_SP

	rsync

	/* Find the next process to run */
	call0 getproc
	
	/* Fall through into switchin */
.align 4
switchin:
	/* Interrupts off */
	rsil a3, 15
	rsync
	/* On entry, a2 is the ptptr of the process to run */

    /* Switch to the swapper stack. */
    movi sp, swapstack_end
    mov.n a15, a2               /* save a2 (the new ptab) in callee-saved register */

    /* Maybe swap in a process. */
    call0 contextswitch
    isync

    /* Make the new process runnable. */
	movi a3, P_RUNNING
	s8i a3, a15, P_TAB__P_STATUS_OFFSET

	/* Restore process stack pointer and return. */

	movi a3, udata
	l32i.n sp, a3, U_DATA__U_SP

	l8ui a4, a3, U_DATA__U_ININTERRUPT

	movi a2, 0
	movi a3, runticks
	s16i a2, a3, 0

	l32i.n a12, sp, 0*4
	l32i.n a13, sp, 1*4
	l32i.n a14, sp, 2*4
	l32i.n a15, sp, 3*4
	l32i.n a3, sp, 4*4
	wsr.sar a3

	l16ui a2, sp, 5*4    /* sometimes the child pid, saved by dofork */

	l32i.n a0, sp, 7*4
	addi sp, sp, 8*4

	/* FIXME: set interrupt state properly based on inint */
	bnez a4, ret_via_int
	rsil a4, 0
	rsync
ret_via_int:
	ret

.global dofork
.text
.align 4
# a2 = ptab of the current process
dofork:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

	addi sp, sp, -8*4
	s32i.n a0, sp, 7*4
	s32i.n a12, sp, 0*4
	s32i.n a13, sp, 1*4
	s32i.n a14, sp, 2*4
	s32i.n a15, sp, 3*4
	rsr.sar a3
	s32i.n a3, sp, 4*4

	/* Store child's pid so the parent can return it */

	l16ui a3, a2, P_TAB__P_PID_OFFSET
	s16i a3, sp, 5*4

	/* Save parent's stack pointer */

	movi a3, udata
	s32i.n sp, a3, U_DATA__U_SP

    /* Copy the current process to the new parent. */

	mov.n a15, a2				/* save a2 in callee-saved register */
    mov.n a14, sp               /* save old stack pointer in callee-saved register */
	movi sp, swapstack_end
    call0 clonecurrentprocess
    mov.n sp, a14

	/* We are now going to become the child; associate the child's p_tab
	 * pointer with the current udata. */

	/* a15 is already the child's p_tab pointer */
    mov.n a2, a15
	movi a3, udata
	call0 makeproc
	
	/* We are now ready to return. */

	movi a2, 0
	movi a3, runticks
	s16i a2, a3, 0

	l32i.n a12, sp, 0*4
	l32i.n a13, sp, 1*4
	l32i.n a14, sp, 2*4
	l32i.n a15, sp, 3*4
	l32i.n a3, sp, 4*4
	wsr.sar a3
	movi a2, 0			/* Child process returns 0 */

	l32i.n a0, sp, 7*4
	addi sp, sp, 8*4
	ret

.global plt_idle
.text
.align 4
plt_idle:
	waiti 0
	ret

.global program_vectors
.text
.align 4
program_vectors:
	ret

/*
 *	Use these instead of the ROM ones. The ROM ones touch data in the
 *	system memory bank we want to 'borrow'.
 */
.global irq_set_mask
.text
.align 4
irq_set_mask:
	rsil a7,2			/* Disable IRQs */
	wsr.intenable a2
	wsr.ps a7
	rsync
	ret.n

/* vim: sw=4 ts=4 et: */
