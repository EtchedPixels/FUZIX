#include "msp430fr5969.h"
#include "kernel.def"
#include "kernel430x.def"

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
; 
; This function can have no arguments or auto variables.

.globl switchout
switchout:
	bic.w #GIE, SR                     ; interrupts off
	mov #0, r12                        ; set return code
	pushm.a #12, r15                   ; save all registers (r15 to r4)
	push SR                            ; ...and the status register
	movx SP, &U_DATA__U_SP             ; save stack pointer
	clra &inint

	;calla #getproc
	jmp switchin                    ; switch in new process
	calla #trap_monitor                ; should never reach here

.globl switchin
switchin:
	bic.w #GIE, SR                     ; interrupts off
	movx &U_DATA__U_SP, SP             ; restore stack pointer
	pop SR                             ; restore status register
	popm.a #12, r15                    ; restore all registers (r15 to r4)
	bis.w #GIE, SR                     ; interrupts on
	reta

