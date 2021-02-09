#include "kernel-lx106.def"

.global udata
.comm udata, UBLOCK_SIZE

#define SWAPSTACK_SIZE 1536
.comm swapstack, SWAPSTACK_SIZE

.global platform_switchout
.global switchin
.text
.align 4
platform_switchout:
	/* Save the callee-saved registers onto the stack in the same order
	 * that dofork does it, so switchin will restore it correctly. */

	addi sp, sp, -8*4
	s32i a0, sp, 7*4
	s32i a12, sp, 0*4
	s32i a13, sp, 1*4
	s32i a14, sp, 2*4
	s32i a15, sp, 3*4
	rsr.sar a3
	s32i a3, sp, 4*4

	/* Save stack pointer */

	movi a3, udata
	s32i sp, a3, U_DATA__U_SP_OFFSET

	/* Find the next process to run */

	call0 getproc
	
	/* Fall through into switchin */
switchin:
	/* On entry, a2 is the ptptr of the process to run */

	/* Is the new process actually swapped out? */

	l16ui a3, a2, P_TAB__P_PAGE_OFFSET
	bnez a3, not_swapped

	/* Switch to the swapper stack and swap in the new process. */

	movi sp, swapstack + SWAPSTACK_SIZE
	mov a15, a2				/* save a2 in callee-saved register */
	call0 swapper
	isync

not_swapped:
	/* Make the new process runnable. */

	movi a3, P_RUNNING
	s8i a3, a15, P_TAB__P_STATUS_OFFSET

	/* Restore process stack pointer and return. */

	movi a3, udata
	l32i sp, a3, U_DATA__U_SP_OFFSET

	movi a2, 0
	movi a3, runticks
	s16i a2, a3, 0

	l32i a12, sp, 0*4
	l32i a13, sp, 1*4
	l32i a14, sp, 2*4
	l32i a15, sp, 3*4
	l32i a3, sp, 4*4
	wsr.sar a3
	l16ui a2, sp, 5*4    /* sometimes the child pid, saved by dofork */

	l32i a0, sp, 7*4
	addi sp, sp, 8*4

	ret

.global dofork
.text
.align 4
# a2 = ptab of the current process
dofork:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

	addi sp, sp, -8*4
	s32i a0, sp, 7*4
	s32i a12, sp, 0*4
	s32i a13, sp, 1*4
	s32i a14, sp, 2*4
	s32i a15, sp, 3*4
	rsr.sar a3
	s32i a3, sp, 4*4

	/* Store child's pid so the parent can return it */

	l16ui a3, a2, P_TAB__P_PID_OFFSET
	s16i a3, sp, 5*4

	/* Save parent's stack pointer */

	movi a3, udata
	s32i sp, a3, U_DATA__U_SP_OFFSET

	/* Save the current process to disk. */

	s32i a2, sp, 6*4
	l32i a2, a3, U_DATA__U_PTAB_OFFSET
	call0 swapout
	l32i a2, sp, 6*4

	/* We are now going to become the child; associate the child's p_tab
	 * pointer with the current udata. */

	/* a2 is already the child's p_tab pointer */
	movi a3, udata
	call0 makeproc
	
	/* We are now ready to return. */

	movi a2, 0
	movi a3, runticks
	s16i a2, a3, 0

	l32i a12, sp, 0*4
	l32i a13, sp, 1*4
	l32i a14, sp, 2*4
	l32i a15, sp, 3*4
	l32i a3, sp, 4*4
	wsr.sar a3
	movi a2, 0			/* Child process returns 0 */

	l32i a0, sp, 7*4
	addi sp, sp, 8*4
	ret

.global platform_idle
.text
.align 4
platform_idle:
	ret

.global program_vectors
.text
.align 4
program_vectors:
	ret

.global syscall_handler_trampoline
.text
.align 4
# a2 = exception frame structure
# a3 = cause
syscall_handler_trampoline:
	s32i sp, a2, 12		/* save user stack to exception frame */
	movi sp, udata + UBLOCK_SIZE /* switch to kernel stack */

	addi sp, sp, -4*4
	s32i a2, sp, 0*4
	s32i a0, sp, 1*4

	call0 syscall_handler

	l32i a2, sp, 0*4
	l32i a0, sp, 1*4
	l32i sp, a2, 12		/* switch back to the user stack */

	ret

/* vim: sw=4 ts=4 et: */

