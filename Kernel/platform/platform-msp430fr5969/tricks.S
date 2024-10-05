#include "msp430fr5969.h"
#include "kernel.def"
#include "kernel430x.def"

SWAPSTACK_SIZE = 128
.section .bss.swapstack
.globl swapstack
swapstack:
	.fill SWAPSTACK_SIZE

; Loads an overlay from high memory to its destination in low memory.
; The address provided in parameters are the *low 16 bits* of the high
; memory addresses (for C compatibility).
proc load_overlay
	; r12 = overlay start
	; r13 = overlay stop
	mov.w #0, r14
	sub.w r12, r13
1:
	movx.w 0x10000(r12), __overlay_start_address(r14)
	incd r12
	incd r14
	cmp.w r14, r13
	jnz 1b
	ret

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().

.globl switchin
proc plt_switchout
	dint                              // interrupts off
	clr r12                           // set return code
	pushm #12, r15                    // save all registers (r15 to r4)
	push SR                           // ...and the status register
	mov SP, &U_DATA__U_SP             // save stack pointer

	call #getproc                     // find a new process to run
switchin:
	// On entry, r12 is the ptptr of the process to run.
	dint                              // interrupts off

	// Compare the swap page 
	cmp #0, P_TAB__P_PAGE_OFFSET(r12)  // get the swap page the process is in
	jnz not_swapped

	// Because we don't support banking, we're using simple.c as our process
	// storage backend, which is a bit simple-minded --- it doesn't handle
	// swapping out processes for us. So, while the swapper will *swap in* a
	// process, we need to deal with the case where it's swapped out manually.
	//
	// Because we're about to overwrite the udata and kstack with the restored
	// process, we need to switch to a special stack. This lets us do the slow
	// bit with interrupts on, which is nice.

	mov #swapstack+SWAPSTACK_SIZE, sp
	eint
	push r12
	mov &U_DATA__U_PTAB, r12          // load old ptptr
	cmp #0, P_TAB__P_PAGE_OFFSET(r12) // has it been swapped out?
	jz 1f                             // if yes, skip
	call #swapout                     // ...if no, swap it out
1:
	mov 0(sp), r12                    // load new ptptr
	call #swapper                     // ...swap it in
	pop r12

not_swapped:
	mov.b #P_RUNNING, P_TAB__P_STATUS_OFFSET(r12) ; mark process as running

	dint
	call #load_overlay_for_syscall    // ensure right overlay is loaded
	mov &U_DATA__U_SP, SP             // restore stack pointer
    clr &runticks                     // reset process run count
	pop SR                            // restore status register
	popm #12, r15                     // restore all registers (r15 to r4)
	eint                              // interrupts on
	ret

; Forks to swap. On entry, r12 is the process pointer.

proc dofork
	dint

	// dofork suspends the parent; the scheduler will resume it again with
	// switchin. So we need to save the parent's state is if it had been
	// saved with switchout.

	pushm #3, r15                     // save r15, r14, r13
	push P_TAB__P_PID_OFFSET(r12)     // save child pid as r12
	pushm #8, r11                     // save r11, r10, r9, r8, r7, r6, r5, r4
	push SR                           // ...and the status register
	mov SP, &U_DATA__U_SP             // save stack pointer

	; Save the parent process to disk.

	push r12
	mov.w &U_DATA__U_PTAB, r12
	eint
	call #swapout
	dint
	pop r12

	; Okay, done. Anything we change now won't affect the parent. Create the
	; child process. First we get rid of all the junk we put on the stack for
	; switchin.

	add #12*2 + 2, sp

	; Now create the child process. (r12 is still the process pointer.)

	call #newproc

	; ...and we're ready to go.

	clr &runticks                      ; reset process run count
	clr r12                            ; child fork returns zero!
	eint                               ; interrupts on
	ret

/* System call entry. The system call number has been pushed onto the stack,
 * above the return address. The four parameters are in r12-r15. */

proc unix_syscall_entry
	dint
	mov.b 2(sp), &U_DATA__U_CALLNO
	mov.w sp, &U_DATA__U_SYSCALL_SP
	mov.w #kstack_top, sp
	mov.w r12, &U_DATA__U_ARGN
	mov.w r13, &U_DATA__U_ARGN1
	mov.w r14, &U_DATA__U_ARGN2
	mov.w r15, &U_DATA__U_ARGN3
	mov.b #1, &U_DATA__U_INSYS
	eint
	call #load_overlay_for_syscall
	call #unix_syscall
	dint
	mov.b #0, &U_DATA__U_INSYS
	mov.w &U_DATA__U_SYSCALL_SP, sp

	; Deliver any pending signals. This may not return, and interrupts will be
	; on afterwards.

	call #deliver_signals
	mov.w &U_DATA__U_RETVAL, r12
	mov.w &U_DATA__U_ERROR, r13
	ret
