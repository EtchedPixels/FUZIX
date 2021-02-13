#include "kernel-armm0.def"

.global udata
.comm udata, UBLOCK_SIZE

.global isr_svcall
.text
.thumb_func
.align 4
# On entry, sp points at the stack frame.
isr_svcall:
    mov r0, sp          /* get address of exception frame (and stack) */

    ldr r1, =(udata + UBLOCK_SIZE) /* switch to kernel stack */
    mov sp, r1
    push {r0}           /* push exception frame address */

    bl syscall_handler  /* call C system call handler */

    pop {r0}            /* retrieve old exception frame address */
    mov sp, r0          /* restore stack */

    ldr r0, =0xFFFFFFF9 /* magic return-from-system-call code */
    bx r0

.global platform_switchout
.global switchin
.text
.align 4
platform_switchout:
    ldr r0, =switchout_msg
    bl panic
switchout_msg:
    .asciz "platform_switchout"

#if 0
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
#endif
	
	/* Fall through into switchin */
switchin:

#if 0
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
#endif

.global dofork
.text
.align 4
# a2 = ptab of the current process
dofork:
    ldr r0, =dofork_msg
    bl panic
dofork_msg:
    .asciz "dofork"
#if 0
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
#endif

/* vim: sw=4 ts=4 et: */

