# asm routines copied from ../platform-rpipico/tricks.S

.cpu cortex-m4

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

    bl get_ustack
    mov r1, r0
    mov r0, sp                  /* get pointer to svc frame */
    mov sp, r1                  /* switch to kernel stack */

    push {r0}
    bl syscall_handler          /* call C system call handler */
    pop {r0}

    mov sp, r0                  /* restore stack */
    pop {r0, r2, r3}            /* r2 is new PC, r3 is the old lr */
    mov lr, r3
    mov r12, r0
    pop {r0, r1}                /* pop return values */
    add sp, #8                  /* discard the old r2 and r3 */
    mov pc, r2                  /* can't use bx as r2 doesn't have the thumb bit set */

.comm svc_pc_stash, 4

.global plt_switchout
.global switchin
.text
.thumb_func
plt_switchout:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

    push {r4, r5, r6, r7, lr}
    mov r1, r8
    mov r2, r9
    mov r3, r10
    mov r4, r11
    mov r5, r12
    push {r1, r2, r3, r4, r5}

	/* Dummy value used as the return value for fork. */

    push {r0}

	/* Save stack pointer */
    mov r0, sp
    bl stash_sp

    /* Find the next process to run. */
    bl getproc /* takes no params, returns ptab ptr */

	/* Fall through into switchin */

    /* On entry, r0 is the ptab ptr of the process to run */
switchin:
    mov r5, r0
	/* Make the new process runnable. */
    bl set_running

    mov r0, r5
    mov r1, #0
    bl pagemap_switch /* takes ptab ptr in r0 and 0 in r1, returns nothing */

    mov r0, r5
    mov r1, #0
    bl set_udata

	/* Restore process stack pointer and return. */
    bl restore_sp
    mov sp, r0

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
    pop {r4, r5, r6, r7, pc}


.global dofork
.text
.thumb_func
# r0 = ptab of the new child process
dofork:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

    push {r4, r5, r6, r7, lr}
    mov r1, r8
    mov r2, r9
    mov r3, r10
    mov r4, r11
    mov r5, r12
    push {r1, r2, r3, r4, r5}

	/* Store child's pid on stack so the parent can return it. */
    mov r5, r0 /* Keep ptab in r5 for later */
    bl ptab_pid
    push {r0}

    mov r0, sp
    bl stash_sp
    mov r0, r5
    mov r1, #1
    bl set_udata /* Should return the new udata */
    mov r6, r0
    bl restore_sp
    mov sp, r0
    mov r0, r5
    mov r1, r6

	/* We are now going to become the child; associate the child's p_tab
	 * pointer with the current udata. */

	/* r0 is already the child's p_tab pointer; r1 points at udata. */
    bl makeproc

	/* We are now ready to return. */

    mov r0, r5
    mov r1, #1 /* death? */
    bl pagemap_switch /* takes ptab ptr in r0 and 0 in r1, returns nothing */

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
    pop {r4, r5, r6, r7, pc}
