#include "kernel-armm0.def"

.global udata
.cpu cortex-m0
.comm udata, UBLOCK_SIZE

.global isr_svcall
.text
.thumb_func
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
.thumb_func
platform_switchout:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

    push {r5, r6, r7, lr}
    mov r1, r8
    mov r2, r9
    mov r3, r10
    mov r4, r11
    mov r5, r12
    push {r1, r2, r3, r4, r5}

	/* Dummy value used as the return value for fork. */

    push {r0}

	/* Save stack pointer */

    ldr r1, =udata
    mov r2, sp
    str r2, [r1, #U_DATA__U_SP_OFFSET]

    /* Find the next process to run. */

    bl getproc

	/* Fall through into switchin */

    /* On entry, r0 is the ptab ptr of the process to run */
switchin:
	/* Is the new process actually swapped out? */

    mov r4, r0              /* save r0 (ptab ptr) in callee-saved registers */
    ldrh r1, [r4, #P_TAB__P_PAGE_OFFSET]
    cmp r1, #0
    bne not_swapped

	/* Switch to the swapper stack and swap in the new process. */

    ldr r1, =__StackTop
    mov sp, r1
    bl swapper
	isb

not_swapped:
	/* Make the new process runnable. */

    mov r0, #P_RUNNING
    str r0, [r4, #P_TAB__P_STATUS_OFFSET]

	/* Restore process stack pointer and return. */

    ldr r0, =udata
    ldr r1, [r0, #U_DATA__U_SP_OFFSET]
    mov sp, r1

    mov r0, #0
    ldr r1, =runticks
    strh r0, [r1]

    pop {r0}                /* pushed pid */
    pop {r1, r2, r3, r4, r5}
    mov r8, r1
    mov r9, r2
    mov r10, r3
    mov r11, r4
    mov r12, r5
    pop {r5, r6, r7, pc}

.global dofork
.text
.thumb_func
# r0 = ptab of the new child process
dofork:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

    push {r5, r6, r7, lr}
    mov r1, r8
    mov r2, r9
    mov r3, r10
    mov r4, r11
    mov r5, r12
    push {r1, r2, r3, r4, r5}

	/* Store child's pid so the parent can return it. */

    ldrh r1, [r0, #P_TAB__P_PID_OFFSET]
    push {r1}

	/* Save parent's stack pointer */

    ldr r1, =udata
    mov r2, sp
    str r2, [r1, #U_DATA__U_SP_OFFSET]

	/* Save the current process to disk. */

    push {r0, r1}
    ldr r0, [r1, #U_DATA__U_PTAB_OFFSET]
    bl swapout
    pop {r0, r1}

	/* We are now going to become the child; associate the child's p_tab
	 * pointer with the current udata. */

	/* r0 is already the child's p_tab pointer; r1 points at udata. */
    bl makeproc
	
	/* We are now ready to return. */

    mov r0, #0
    ldr r1, =runticks
    strh r0, [r1]

    pop {r0}            /* pushed pid */
    mov r0, #0          /* Child process returns 0 */
    pop {r1, r2, r3, r4, r5}
    mov r8, r1
    mov r9, r2
    mov r10, r3
    mov r11, r4
    mov r12, r5
    pop {r5, r6, r7, pc}

/* vim: sw=4 ts=4 et: */

