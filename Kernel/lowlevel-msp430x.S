#include "platform-msp430fr5969/kernel.def"
#include "kernel430x.def"

.section ".lowtext"
.globl interrupt_handler
interrupt_handler:
	/* Interrupts are off here and we're running on... a... stack. This
	 * may be the user process or the kernel's. We haven't saved any
	 * registers yet. */

	movx.w SP, &istack_switched_sp        ; save old stack (16 bit!)
	mov #istack_top, SP                ; load new one

	pushm.a #13, r15                   ; save all registers (r15 to r3)

	calla #platform_interrupt

	popm.a #13, r15                    ; restore all registers (r15 to r3)
	
	movx.w &istack_switched_sp, SP        ; restore stack
	reti

proc doexec
	dint
	#if 0
        di
        call map_process_always

        pop bc ; return address
        pop de ; start address

        ld hl, (U_DATA__U_ISP)
        ld sp, hl      ; Initialize user stack, below main() parameters and the environment

        ; u_data.u_insys = false
        xor a
        ld (U_DATA__U_INSYS), a

        ex de, hl

	; for the relocation engine - tell it where it is
	ld iy, #PROGLOAD
        ei
        jp (hl)
#endif
	jmp .
