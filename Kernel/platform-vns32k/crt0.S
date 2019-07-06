#include "../kernel-ns32k.def"
#
#	Called with interrupts off and in supervisor mode
#
		jump start
		.align 2
module:
sbase:		.long 0
		.long 0
		.long 0
		.long 0
inttab:
		# Non vectored external IRQ (all in our case)
		.word 0
		.word interrupt_handler
		# Non maskable interrupt (not wired)
		.word 0
		.word nmi_handler
		.word 0
		.word abort_handler
		.word 0
		.word slave_handler
		.word 0
		.word illegal_handler
		.word 0
		.word syscall_handler
		.word 0
		.word divzero_handler
		.word 0
		.word flag_handler
		.word 0
		.word breakpoint_handler
		.word 0
		.word trace_handler
		.word 0
		.word undefined_handler
		.long 0
		.long 0
		.long 0
		.long 0
		.long 0
		# Vectored interrupts follow (non used)

nmi_handler:
		rett 0
abort_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 0,tos
do_exception:
		sprw psr,r0
		andw 0xF7FF,r0		# Disable interrupts
		lprw psr,r0
		sprd sp,r0
		movd r0,tos
		bsr _exception
		cmpd tos,tos		# Drop 8 bytes
		# Called function may have played with the return stack
		# It may also kill, switch and never return
		exit [r0,r1,r2,r3,r4,r5,r6,r7]
		rett 0
slave_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 1,tos
		br do_exception
illegal_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 2,tos
		br do_exception
divzero_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 3,tos
		br do_exception
flag_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 4,tos
		br do_exception
breakpoint_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 5,tos
		br do_exception
trace_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 6,tos
		br do_exception
undefined_handler:
		enter [r0,r1,r2,r3,r4,r5,r6,r7],0
		movqd 1,tos
		br do_exception
syscall_handler:
		sprw psr,r1
		andw 0xF7FF,r1		# Disable interrupts
		lprw psr,r1
		enter [r6],0
		movd _udata_shadow(pc),r6
		movb r0,U_DATA__U_CALLNO(r6)
		movqb 1,U_DATA__U_INSYS(r6);
		sprd usp,r0
		movd 8(ro),U_DATA__U_ARGN(r6)
		movd 12(ro),U_DATA__U_ARGN1(r6)
		movd 16(ro),U_DATA__U_ARGN2(r6)
		movd 20(ro),U_DATA__U_ARGN3(r6)
		movqb 1,_kernel_flag(pc)
		sprw psr,r0
		orw 0x0800,r0		# Allow interrupts
		lprw psr,r0
		bsr _unix_syscall
		sprw psr,r1
		andw 0xF7FF,r1		# Disable interrupts
		lprw psr,r1
		movqb 0,U_DATA__U_INSYS(r6)
		movqb 0,_kernel_flag(pc)				
		movw U_DATA__U_ERROR(r6),r1
		cmpqw 0,r1
		bne sysc_err
		movd U_DATA__U_RETVAL(r6),r0
sysc_sig:
		cmpqb 0,U_DATA__U_CURSIG(r6)
		bne return_via_signal
		exit [r6]
		rett 0
sysc_err:
		movqd -1,r0
		br sysc_sig
return_via_signal:
		# FIXME concoct new stack frames
		

interrupt_handler:
		enter [r0,r1,r6],0
		movd _udata_shadow(pc),r6
		movqb 1,U_DATA__U_ININTERRUPT(r6)
		bsr _platform_interrupt
		movqb 0,U_DATA__U_ININTERRUPT(r6)
		cmpqb 0,U_DATA__U_INSYS(r6)
		bne no_preempt
		cmpqb 0,_need_resched(pc)
		bne no_preempt

		# Pre-emption

		movd U_DATA__U_PTAB(r6),r0
		cmpqb P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)
		bne no_ready
		movqb P_READY,P_TAB__P_STATUS_OFFSET(r0)
no_ready:
		bsr _switchout
no_preempt:
		cmpb 0,U_DATA__U_CURSIG(r6)
		beq no_signal
		# FIXME concoct new stack frames
no_signal:
		exit [r0,r1,r6]
		reti


#
#	Need to wipe BSS etc once we figure out our preferred boot method
#
		.globl __end
		.globl __bss_start

start:
		# Set up the IRQ vector pointer
		addr inttab,r0
		lprd intbase,r0
		# And modules - not that we use them
		addr module,r0
		lprw mod,r0
		movd sbase,r0
		lprd sb,r0
		# Save the CPU configuration
		sprd cfg,r0
		movd r0,_sysconfig(pc)
		addr __bss_start,r0
		addr __end,r1
wipebss:
		movqd 0,0(r0)
		addqd 4,r0
		cmpd r1,r0
		blt wipebss		# Check compare direction is right

		/* FIXME: hard coded ugly */
		addr _udata_block+1016,r0
		lprd sp,r0
		/* udata global */
		addr _udata_block,r6
		bsr init_early
		bsr init_hardware
		bsr _fuzix_main
		sprw psr,r0		# Interrupts back off
		andw 0xF7FF,r0
		lprw psr,r0
stop:		br stop
