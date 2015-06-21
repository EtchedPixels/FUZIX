#include "kernel430x.def"

.section ".lowtext"
.globl interrupt_handler
interrupt_handler:
	/* Interrupts are off here and we're running on... a... stack. This
	 * may be the user process or the kernel's. We haven't saved any
	 * registers yet. */

	mov SP, &istack_switched_sp        ; save old stack (16 bit!)
	mov #istack_top, SP                ; load new one

	pushm.a #13, r15                   ; save all registers (r15 to r3)

	calla #platform_interrupt

	popm.a #13, r15                    ; restore all registers (r15 to r3)
	
	mov &istack_switched_sp, SP        ; restore stack
	reti

