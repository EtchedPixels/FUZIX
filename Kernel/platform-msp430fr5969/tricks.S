#include "msp430fr5969.h"
#include "kernel.def"
#include "kernel430x.def"

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().

.globl switchin
proc switchout
	dint                               ; interrupts off
	clr r12                            ; set return code
	pushm.a #12, r15                   ; save all registers (r15 to r4)
	push SR                            ; ...and the status register
	mov SP, &U_DATA__U_SP              ; save stack pointer
	clr &inint

	calla #getproc                     ; find a new process to run
switchin:
	dint                               ; interrupts off
	mov.w #U_DATA__U_PTAB, r12
	mov.b #P_RUNNING, P_TAB__P_STATUS_OFFSET(r12) ; mark process as running

	movx &U_DATA__U_SP, SP             ; restore stack pointer
    clr &runticks                      ; reset process run count
	pop SR                             ; restore status register
	popm.a #12, r15                    ; restore all registers (r15 to r4)
	eint                               ; interrupts on
	reta

; Forks to swap. On entry, r12 is the process pointer.

proc dofork
	dint

	; dofork suspends the parent; the scheduler will resume it again with
	; switchin. So we need to save the parent's state is if it had been
	; saved with switchout.

	pushm.a #12, r15                   ; save all registers (r15 to r4)
	push SR                            ; ...and the status register
	mov SP, &U_DATA__U_SP              ; save stack pointer
	clr &inint

	; Save the parent process to disk.

	pushx.a r12
	movx.a &U_DATA__U_PTAB, r12
	calla #swapout
	popx.a r12

	; Okay, done. Anything we change now won't affect the parent. Create the
	; child process. First we get rid of all the junk we put on the stack for
	; switchin.

	add #12*4 + 2, sp

	; Now create the child process. (r12 is still the process pointer.)

	calla #newproc

	; ...and we're ready to go.

	clr &runticks                      ; reset process run count
	clr r12                            ; child fork returns zero!
	eint                               ; interrupts on
	reta
