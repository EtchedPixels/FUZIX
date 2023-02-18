#include "../kernel-ns32k.def"
/*
 *	Lots left to fill in
 */

		.globl _plt_reboot
		.globl init_early
		.globl init_hardware
		.globl _program_vectors
		.globl outchar
		.globl _plt_monitor
		.globl _udata_block

#
#	Put early as needs to be in the low 64K
#

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
		bsr _plt_interrupt
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

_plt_reboot:
_plt_monitor:
		sprw psr,r0		# Interrupts back off
		andw 0xF7FF,r0
		lprw psr,r0
		br _plt_monitor

init_early:
		addr _udata_block(pc),_udata_shadow(pc)
		addr inttab,r0
		lprd intbase,r0
		addr module,r0
		lprw mod,r0
		movd sbase,r0
		lprd sb,r0
		ret 0


#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now)
		movw 1024,_ramsize(pc)
		movw 1024-64,_procmem(pc)		# guesses for now

		bsr _program_vectors

# TODO timer enable etc

		ret 0

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
	    movb 0xF0018A(pc),r1
	    andb 0x20,r1
	    beq outcharw
	    movb r0,0xF00180(pc)
	    movw tos,r1
	    ret 0

	    .data
	    .globl _kernel_flag

_kernel_flag:
	    .byte 0

	    .globl _copy_blocks
	    .globl _swap_blocks
	    .globl _set_cpu_type
	    .globl _sys_cpu
	    .globl _sys_cpu_feat
	    .globl _vdso

# Stuff to tackle - block copiers
_copy_blocks:
_swap_blocks:

# Stuff to tackle CPU setup. Moves to lowlevel ?

_set_cpu_type:
	    ret  0
_sys_cpu:
	    .byte 0
_sys_cpu_feat:
	    .byte 0
_vdso:
	    .word 0,0,0,0,0,0,0,0
	    .word 0,0,0,0,0,0,0,0
	    .word 0,0,0,0,0,0,0,0
	    .word 0,0,0,0,0,0,0,0
