#include "kernel-armm0.def"

.global udata
.cpu cortex-m0

udata = progbase

.text
.thumb_func
.global platform_idle
platform_idle:
    wfi
    bx lr

.text
.thumb_func
fatal_exception_handler_trampoline:
    mrs r0, IPSR
    mov r1, sp
    push {r0, r1, r4, r5, r6, r7}
    mov r0, r8
    mov r1, r9
    mov r2, r10
    mov r3, r11
    push {r0, r1, r2, r3}
    mov r0, sp
    ldr r1, =__StackTop
    mov sp, r1
    bl fatal_exception_handler

.global isr_hardfault
isr_hardfault = fatal_exception_handler_trampoline
    
.global isr_svcall
.text
.thumb_func
# On entry, sp points at the stack frame.
isr_svcall:
    # This is executing in handler mode. We can't run system calls in handler
    # mode because weird things happen with interrupts, so instead we fake a call
    # to isr_svcall_user and return back to thread mode.

    ldr r0, [sp, #6*4]  /* Load old PC */
    ldr r1, =svc_pc_stash
    str r0, [r1]        /* Save old PC */
    ldr r0, =isr_svcall_user /* Load user-mode ISR */
    str r0, [sp, #6*4]  /* Update old PC */
    bx lr               /* Return out of Handler mode */

isr_svcall_user:
    # At this point, we're running in thread mode. All registers (with the
    # exception of PC) are as they were when the user program called svc,
    # Essentially, they called us as if they were a normal procedure call,
    # except we need to preserve lr.

    push {r0, r1, r2, r3}       /* push parameters */
    mov r0, r12
    ldr r1, =svc_pc_stash
    ldr r1, [r1]
    push {r0, r1, lr}           /* save r12, PC, LR */

    mov r0, sp                  /* get pointer to svc frame */
    ldr r1, =(udata + UDATA_SIZE_ASM) /* switch to kernel stack */
    mov sp, r1

    push {r0}
    bl syscall_handler          /* call C system call handler */
    pop {r0}

    mov sp, r0                  /* restore stack */
    bl deliver_signals          /* may not return if signal handler longjmps out */

    pop {r0, r2, r3}            /* r2 is new PC, r3 is the old lr */
    mov lr, r3
    mov r12, r0
    pop {r0, r1}                /* pop return values */
    add sp, #8                  /* discard the old r2 and r3 */
    mov pc, r2                  /* can't use bx as r2 doesn't have the thumb bit set */

.comm svc_pc_stash, 4

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
    ldr r1, =__StackTop     /* switch to swapper stack */
    mov sp, r1

    mov r4, r0              /* save r0 (ptab ptr) in callee-saved registers */
    bl contextswitch        /* may swap a process in */
	isb

	/* Make the new process runnable. */

    mov r0, #P_RUNNING
    strb r0, [r4, #P_TAB__P_STATUS_OFFSET]

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

	/* Copy the current process to the new parent. */

    push {r0, r1}
    mov r4, sp          /* save SP in callee-saved register */
    ldr r1, =__StackTop /* switch to the swapper stack */
    mov sp, r1
    bl clonecurrentprocess
    isb
    mov sp, r4          /* restore old stack */
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

// vim: sw=4 ts=4 et:

