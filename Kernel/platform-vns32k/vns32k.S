
#include "../kernel-ns32k.def"
/*
 *	Lots left to fill in
 */

		.globl _platform_reboot
		.globl init_early
		.globl init_hardware
		.globl _program_vectors
		.globl outchar
		.globl _platform_monitor
		.globl _udata_block

_platform_reboot:
_platform_monitor:
		sprw psr,r0		# Interrupts back off
		andw 0xF7FF,r0
		lprw psr,r0
		br _platform_monitor

init_early:
		addr _udata_block(pc),_udata_shadow(pc)
		ret 0


#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now)
		movw 512,_ramsize(pc)
		movw 512-64,_procmem(pc)		# guesses for now

		bsr _program_vectors

# TODO interrupt table etc
#	    move.l #timer_irq,$78(a0)
#	    move.l #mmu_fault,$7C(a0)
#
# TODO timer enable etc
#
		ret 0

timer_irq:
		save [r0,r1,r6]
		# C will save and restore r2+
		# and we don't use FP registers in kernel
		movd _udata_shadow(pc),r6	# set up the register global
		movb 1,U_DATA__U_ININTERRUPT(r6)
		bsr _platform_interrupt
		movb 0,U_DATA__U_ININTERRUPT(r6)

		# Timer re-enable ?

		cmpb 0,U_DATA__U_INSYS(r6)
		bne no_preempt
		cmpb 0,_need_resched(pc)
		beq no_preempt
		#
		# Vanish into the scheduler. Some other task will pop back out
		# and eventually we'll re-appear here and continue.
		# FIXME: check IRQ masking
		#
		# We might be in STOPPED state in which case do not go back
		# to ready !
		movd U_DATA__U_PTAB(r6),r0
		cmpb P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)
		bne no_ready
		movb P_READY,P_TAB__P_STATUS_OFFSET(r0)
no_ready:
		movd U_DATA__U_PTAB(r6),r0
		bsr _switchout
no_preempt:
		cmpb 0,U_DATA__U_CURSIG(r6)
		beq no_signal
		# TODO - signal entry into C helper code
		exit [r0,r1,r6]
		reti
no_signal:
		exit [r0,r1,r6]
		reti

#
#	Nothing to do in NS32K - all set up once at boot
#
_program_vectors:
		ret 0

#
#	We do no banking so we need to do nothing here.
#
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	    ret 0


# outchar: Wait for UART TX idle, then print the char in r0

outchar:
	    movw r1,tos
outcharw:
#
#	68B50 ACIA at 0xFFF00
#
	    movb 0xFFF00(pc),r1
	    andb 2,r1
	    beq outcharw
	    movb r0,0xFFF01(pc)
	    movw tos,r1
	    ret 0

	    .data
	    .globl _kernel_flag

_kernel_flag:
	    .byte 0
