#include "kernel-esp32.def"

.global udata
.align 16
.comm udata, UBLOCK_SIZE

#define SWAPSTACK_SIZE 1536
.align 16
.comm swapstack, SWAPSTACK_SIZE

.global istack

#define ISTACK_SIZE 516		/* 512 for the stack 4 for the previous stack save */
.align 16
.comm istack, ISTACK_SIZE

.global platform_switchout
.global switchin
.text
.align 4
platform_switchout:
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
switchin:
	/* Interrupts off */
	rsil a3, 15
	rsync
	/* On entry, a2 is the ptptr of the process to run */

	/* Is the new process actually swapped out or did we run last ? */
	l16ui a3, a2, P_TAB__P_PAGE_OFFSET
	bnez a3, not_swapped

	/* Switch to the swapper stack and swap in the new process. */
	movi sp, swapstack + SWAPSTACK_SIZE
	mov.n a15, a2				/* save a2 in callee-saved register */

	/* If the in memory image is a corpse we don't need to write it back out */
	movi a3, udata
	l16si a4, a3, U_DATA__U_PAGE		/* sign extend it so we can do a simple -1 test */
	addi a4, a4, 1
	beqz a4, hes_dead_jim

	/* Swap out the old process */
	l32i.n a2, a3, U_DATA__U_PTAB
	call0 swapout

	mov.n a2, a15
	/* And swap in the new one */
hes_dead_jim:
	call0 swapper
	isync

not_swapped:

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

	/* Save the current process to disk. */

	s32i.n a2, sp, 6*4
	l32i.n a2, a3, U_DATA__U_PTAB
	call0 swapout
	l32i.n a2, sp, 6*4

	/* We are now going to become the child; associate the child's p_tab
	 * pointer with the current udata. */

	/* a2 is already the child's p_tab pointer */
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

.global platform_idle
.text
.align 4
platform_idle:
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

