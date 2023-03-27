#include "kernel-ns32k.def"
#
#	Low level support for NS32K
#
	.globl	___hard_di
	.globl	___hard_ei
	.globl	___hard_irqrestore
	.globl	_doexec

	.globl	outstring
	.globl	outstringhex
	.globl	outr0hex
	.globl	outcharhex

	.globl	_enable_icache
	.globl	_flush_icache
	.globl	_enable_dcache
	.globl	_flush_dcache

	.globl	_get_usp
	.globl	_set_usp

	.globl	_udata_shadow

	.globl	nmi_handler
	.globl	abort_handler
	.globl	slave_handler
	.globl	illegal_handler
	.globl	divzero_handler
	.globl	flag_handler
	.globl	breakpoint_handler
	.globl	trace_handler
	.globl	undefined_handler
	.globl	syscall_handler
	.globl	interrupt_handler

nmi_handler:
	rett	0
abort_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	0,tos
do_exception:
	movd	_udata_shadow,r6
	sprw	psr,r0
	andw	0xF7FF,r0		# Disable interrupts
	lprw	psr,r0
	sprd	sp,tos
	bsr	_exception
	adjspb	-8
	movb	U_DATA__U_CURSIG(r6),r0
	cmpqb	0,r0
	bne	with_signal
	exit	[r0,r1,r2,r3,r6]
	# Retry the instruction
	rett	0

slave_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	1,tos
	br	do_exception
illegal_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	2,tos
	br	do_exception
divzero_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	3,tos
	br	do_exception
flag_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	4,tos
	br	do_exception
breakpoint_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	5,tos
	br	do_exception
trace_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	6,tos
	br	do_exception
undefined_handler:
	enter	[r0,r1,r2,r3,r6],0
	movqd	1,tos
	br	do_exception
syscall_handler:
	sprw	psr,r1
	andw	0xF7FF,r1		# Disable interrupts
	lprw	psr,r1
	# Top of stack must match what is expected by fork()
	# see dofork.
	enter	[r6],0
	movd	_udata_shadow(pc),r6
	sprd	usp,r1
	movd	r1,U_DATA__U_SYSCALL_SP(r6)	# For brk checking
	movb	r0,U_DATA__U_CALLNO(r6)
	movqb	1,U_DATA__U_INSYS(r6);
	sprd	usp,r0
	movd	4(r0),U_DATA__U_ARGN(r6)
	movd	8(r0),U_DATA__U_ARGN1(r6)
	movd	12(r0),U_DATA__U_ARGN2(r6)
	movd	16(r0),U_DATA__U_ARGN3(r6)
	movqb	1,_kernel_flag(pc)
	sprw	psr,r0
	orw	0x0800,r0		# Allow interrupts
	lprw	psr,r0
	bsr	_unix_syscall
	sprw	psr,r1
	andw	0xF7FF,r1		# Disable interrupts
	lprw	psr,r1
	movqb	0,U_DATA__U_INSYS(r6)
	movqb	0,_kernel_flag(pc)
	movzwd	U_DATA__U_ERROR(r6),r1
	cmpqw	0,r1
	bne	sysc_err
	movd	U_DATA__U_RETVAL(r6),r0
sysc_sig:
	cmpqb	0,U_DATA__U_CURSIG(r6)
	bne	return_via_signal
	exit	[r6]
	addqd	1,0(sp)		# Move past the SVC
	rett	0
sysc_err:
	movqd	-1,r0
	br	sysc_sig

	.globl return_via_signal	# For debug

return_via_signal:
	#	Shuffle frames to look like an IRQ and
	#	save the return code etc the user expects
	exit	[r6]
	enter	[r0,r1,r2,r3,r6],0
	movd	_udata_shadow(pc),r6
	br	with_signal

interrupt_handler:
	# Save what the C compiler will not
	# TODO: Review ABI to check r2/r3 etc
	enter	[r0,r1,r2,r3,r6],0
	movd	_udata_shadow(pc),r6
	movqb	1,U_DATA__U_ININTERRUPT(r6)
	bsr	_plt_interrupt
	movqb	0,U_DATA__U_ININTERRUPT(r6)
	cmpqb	0,U_DATA__U_INSYS(r6)
	bne	no_preempt
	cmpqb	0,_need_resched(pc)
	beq	no_preempt

	# Pre-emption - as we are running with a separate user
	# stack we don't have too much to do here
	# TODO: flag cpu hogs

	movd	U_DATA__U_PTAB(r6),r0
	cmpqb	P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)
	bne	no_ready
	movqb	P_READY,P_TAB__P_STATUS_OFFSET(r0)
no_ready:
	bsr	_switchout
no_preempt:
	cmpb	0,U_DATA__U_CURSIG(r6)
	bne	with_signal
no_signal:
	exit	[r0,r1,r2,r3,r6]
	rett	0
#
#	Exception handling. Push a frame onto the user stack
#	and doctor the trap return stack.
#
#	Right now the supervisor stack looks like this
#
#	PSR/MOD
#	Return Address
#	FP	(for kstack)
#	R6
#	R3
#	R2
#	R1
#	R0	<- SP
#
#	If we ever do protected mode we'll need to handle these
#	faulting.
#
with_signal:
	movzbd	U_DATA__U_CURSIG(r6),r0
	movd	U_DATA__U_SIGVEC(r6)[r0:d],r1
	movqd	0,U_DATA__U_SIGVEC(r6)[r0:d]
	cmpqd	0,r1
	beq	no_signal
	sprd	usp,r0
	subd	36,r0
	# Now fill in the new frame
	# Swap the PC and flags over so we can use ret easily
	movd	24(sp),32(r0)	# Copy the return address from the rett
	movd	28(sp),28(r0)	# and the status flags
	# Now copy the restore frame
	movd	16(sp),24(r0)	# R6
	movd	12(sp),20(r0)	# R3
	movd	8(sp),16(r0)	# R2
	movd	4(sp),12(r0)	# R1
	movd	0(sp),8(r0)	# R0
	# Now set up the signal frame
	movd	U_DATA__U_CURSIG(r6),4(r0)
	# Return from the signal via the stub
	movd	U_DATA__U_CODEBASE(r6),0(r0)
	# Clear the signal as handled
	movqb	0,U_DATA__U_CURSIG(r6)
	# Update our rett frame to the signal vector
	movd	r1,24(sp)
	lprd	usp,r0
	exit	[r0,r1,r2,r3,r6]
	rett	0		# into the signal handler

_doexec:
	enter	[],0
	movd	8(fp),r0
	# Put the supervisor stack back where we want it
	addr	UBLOCK_SIZE(r6),r1
	lprd	sp,r1
	lprd	usp,U_DATA__U_ISP(r6)
	# Interrupts off for the momentary period while insys is wrong
	sprw	psr,r2
	andw	0xF7FF,r2
	lprw	psr,r2
	movqb	0,U_DATA__U_INSYS(r6)
	movqd	0,r2
	movqd	0,r3
	movqd	0,r4
	movqd	0,r5
	movqd	0,r6
	movqd	0,r7
	# We want U S I set (user mode, user stack, interrupt on
	movw	0x0B00,tos	# PSR and if we do DE mode module
	sprw	mod,tos
	movd	r0,tos
	# TODO: clear fp registers if we do an FPU supporting part
	rett	0			# To user space
#
#	Interrupt control. The CPU can support fancy vectored interrupts
#	in which case you probably want to just mask the timer and stuff
#
___hard_di:
	sprw	psr,r0
	movw	r0,r1
	andw	0x800,r0
	andw	0xF7FF,r1
	lprw	psr,r1
	ret	0
___hard_ei:
	sprw	psr,r0
	movw	r0,r1
	andw	0x800,r0
	orw	0x800,r1
	lprw	psr,r1
	ret	0
___hard_irqrestore:
	enter	[],0
	sprw	psr,r0
	andw	0xF7FF,r0
	orw	8(fp),r0
	lprw	psr,r0
	exit	[]
	ret	0

outstring:
	enter	[r0,r1,r2],0
	movb	0(r1),r0
	cmpqb	0,r0
	beq	outdone
	jsr	outchar
	addqd	1,r1
	br	outstring
outdone:
	exit	[r0,r1,r2]
	ret	0

outstringhex:
	enter	[r0,r1],0
outstringhexl:
	movb	0(r1),r0
	cmpqb	0,r0
	beq	outdone
	bsr	outcharhex
	addqd	1,r1
	br	outstringhexl

outcharhex:
	movb	r0,tos
	bsr	outonehex
	movb	tos,r0
outonehex:
	andb	0xf0,r0
	lshb	-4,r0
	addb	'0',r0
	cmpb	'9',r0
	ble	digit
	addqb	7,r0
digit:	br	outchar(pc)
			
outr0hex:	
	enter	[r0,r1,r2],0
	movd	r0,r2
	lshd	-12,r0
	bsr	outcharhex
	movd	r2,r0
	lshd	-8,r0
	bsr	outcharhex
	movw	r2,r0
	lshw	-4,r0
	bsr	outcharhex
	movb	r2,r0
	bsr	outcharhex
	exit	[r0,r1,r2]
	ret	0

	.globl	_copy_blocks
	.globl	_swap_blocks
	.globl	_set_cpu_type
	.globl	_sys_cpu
	.globl	_sys_cpu_feat
	.globl	_vdso
	.globl	_kernel_flag

#
#	Copy blocks of memory for flat modes
#
_copy_blocks:
	enter	[r2,r3],0
	movd	8(fp),r2
	movd	12(fp),r1
	movd	16(fp),r3
copyloop:
	movd	512/4,r0
	movsd
	addqw	-1,r3
	cmpw	0,r3
	bne	copyloop
	exit	[r2,r3]
	ret	0

_swap_blocks:
	# This one is trickier as we've got no handy
	# block operation for it
	# Definitely optimization candidate TODO
	enter	[r2,r3,r4],0
	movd	8(fp),r1
	movd	12(fp),r2
	movd	16(fp),r3
swappitb:
	movw	512/4,r0
swappit:
	movd	0(r1),r4
	movd	0(r2),0(r1)
	movd	r4,0(r2)
	addqd	4,r1
	addqd	4,r2
	addqw	-1,r0
	cmpw	0,r0
	bne	swappit
	addqd	-1,r3
	cmpd	0,r3
	bne	swappitb
	exit	[r2,r3,r4]
	ret	0

# Stuff to tackle CPU setup.
#
#	There are small differences at system level but for user level
#	it's not clear we need to care about anything for the chips we
#	are interested in (332 and 532 do have more user stuff)
#
_set_cpu_type:
	ret	0
# TODO: the signal return stub will need to go here.
_vdso:
	# We enter here when the signal handler returns
	# The handle rsohuls have preserved the high registers
	# and will have put FP back as it was on entry
	# Throw the signal number
	adjspb	-4
	# Recover the scratch registers
	restore	[r0,r1,r2,r3,r6]
	# Throw away the mod from the rett stack copy
	cmpqw	0,tos
	# Recover the flag status
	# lprw	upsr,tos
	# Assembler seems borked on this, so it's
	# gen | short | 11011 | i
	# 10111 0000 11011 01
	.word	0xB86D
	# To the previously executing code
	# Allowing for the fact rett still points to the svc byte
	addqd	1,0(sp)
	ret	0

	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0

	.data
	.align	2
_udata_shadow:
	.long	0
_kernel_flag:
	.byte	0
_sys_cpu:
	.byte	0
_sys_cpu_feat:
	.byte	0
