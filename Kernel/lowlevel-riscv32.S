#include "kernel-riscv32.def"
#
#	Low level support for RISCV32
#
	.globl	doexec

	.globl	outstring
	.globl	outstringhex

	.globl	udata_shadow

	.globl	syscall_handler
	.globl	interrupt_handler


#if 0
syscall_handler:
	.option	nopic
	.option	push
	.option norelax
	la	gp,__global_pointer$
	.option	pop
	la	tp,udata_shadow
	la	tp,(tp)
	sw	sp,U_DATA__U_SYSCALL_SP(tp)
	addi	sp,tp, UBLOCK_SIZE - 12
	# Top of stack must match what is expected by fork()
	# see dofork.
	sw	ra,8(sp)
	sw	gp,4(sp)
	sw	tp,0(sp)

	jal	__hard_di
		
	sb	t0,U_DATA__U_CALLNO(tp)
	li	t0,1
	sb	t0,U_DATA__U_INSYS(tp)
	sw	a1,U_DATA__U_ARGN(tp)
	sw	a2,U_DATA__U_ARGN1(tp)
	sw	a3,U_DATA__U_ARGN2(tp)
	sw	a4,U_DATA__U_ARGN3(tp)
	sb	t0,%lo(kernel_flag)(gp)

	jal	__hard_ei
	jal	unix_syscall
	jal	__hard_di

	sb	zero,U_DATA__U_INSYS(tp)
	sb	zero,%lo(kernel_flag)(gp)
	lh	t0, U_DATA__U_ERROR(tp)
	beq	zero,t0,sysc_err
	lw	a1,U_DATA__U_RETVAL(tp)
sysc_sig:
	lw	t1,U_DATA__U_CURSIG(tp)
	beq	zero,t1,return_via_signal
	mret
sysc_err:
	li	a1,-1
	j	sysc_sig

# Move this one to C ??

	.globl return_via_signal	# For debug

return_via_signal:
	# TOD0 - need to add to user frame and twiddle mret
	# TODO - save retval/error
	jal	deliver_signals
	# TODO - restore retval/error
	mret
#endif

#if 0
# As we are not expecting fancy MMU stuff we put some things on
# the caller stack. With an MMU we'd need to use mscratch and switch
# stacks.
#
# TODO - check and distinguish sources including traps somewhere
#
interrupt_handler:
	addi	sp,sp,-68
	sw	ra,0(sp)
	sw	a1,4(sp)
	sw	a2,8(sp)
	sw	a3,12(sp)
	sw	a4,16(sp)
	sw	a5,20(sp)
	sw	a6,24(sp)
	sw	a7,28(sp)
	sw	t0,32(sp)
	sw	t1,36(sp)
	sw	t2,40(sp)
	sw	t3,44(sp)
	sw	t4,48(sp)
	sw	t5,52(sp)
	sw	t6,56(sp)
	sw	gp,60(sp)
	sw	tp,64(sp)

	.option	push
	.option norelax
	la	gp,__global_pointer$
	.option	pop
	la	tp,udata_shadow
	li	t1,1
	sb	t1,U_DATA__U_ININTERRUPT(tp)
	jal	plt_interrupt
	sb	zero,U_DATA__U_ININTERRUPT(tp)

	lb	t0,U_DATA__U_INSYS(tp)
	bne	zero,t0,no_preempt
	lb	t0,%lo(need_resched)(gp)
	beq	zero,t0,no_preempt

	# Pre-emption - as we are running with a per thread combined i/k
	# stack we don't have too much to do here. If we want to save
	# memory we might revisit that later

	# TODO: flag cpu hogs

	lw	t0,U_DATA__U_PTAB(tp)
	li	t1,P_RUNNING
	lb	t2,P_TAB__P_STATUS_OFFSET(t0)
	bne	t1,t2,no_ready
	li	t1,P_READY
	sb	t1,P_TAB__P_STATUS_OFFSET(t0)
no_ready:
	jal	switchout
no_preempt:
	lb	t0,U_DATA__U_CURSIG(tp)
	bne	zero,t0,with_signal
no_signal:
	lw	tp,64(sp)
	lw	gp,60(sp)
	lw	t6,56(sp)
	lw	t5,52(sp)
	lw	t4,48(sp)
	lw	t3,44(sp)
	lw	t2,40(sp)
	lw	t1,36(sp)
	lw	t0,32(sp)
	lw	a7,28(sp)
	lw	a6,24(sp)
	lw	a5,20(sp)
	lw	a4,16(sp)
	lw	a3,12(sp)
	lw	a2,8(sp)
	lw	a1,4(sp)
	lw	ra,0(sp)
	mret

with_signal:
	#	This is wrong FIXME
	#	need to add a stub to the user space and dork with mret
	#	destination
	jal	deliver_signals
	j	no_signal
#endif

doexec:
	jal	__hard_di
	# Put the supervisor stack back where we want it
	addi	sp, tp, UBLOCK_SIZE
	lw	sp,U_DATA__U_ISP(tp)
	sb	zero, U_DATA__U_INSYS(tp)
	#	TODO clear registers ?
	#	TODO set user mode
	jal	__hard_ei
	jalr	a1

outstring:
	mv	t1,ra		# save return address
	lb	t0,(a1)
	addi	a1,a1,1
	beq	zero,t0,outdone
	jal	outchar
	j	outstring
outdone:
	mv	ra,t1
	jr	ra

	.globl	copy_blocks
	.globl	swap_blocks
	.globl	set_cpu_type
	.globl	sys_cpu
	.globl	sys_cpu_feat
	.globl	vdso
	.globl	kernel_flag

#
#	Copy blocks of memory for flat modes. Needs optimizing
#
copy_blocks:
	li	t0,512/4
copyloop:
	lw	t1,(a2)
	sw	t1,(a1)
	addi	a1,a1,4
	addi	a2,a2,4
	addi	t0,t0,-1
	bne	t0,zero,copyloop
	addi	a3,a3,-1
	bne	a3,zero,copy_blocks
	jalr	ra

swap_blocks:
	# Definitely optimization candidate TODO
	li	t0,512/4
swappit:
	lw	t1,(a1)
	lw	t2,(a2)
	sw	t1,(a2)
	sw	t2,(a1)
	addi	a1,a1,4
	addi	a2,a2,4
	addi	t0,t0,-1	
	bne	t0,zero,swappit
	addi	a3,a3,-1
	bne	a3,zero,swap_blocks
	jalr	ra

# Stuff to tackle CPU setup.
#
#	There are small differences at system level but for user level
#	it's not clear we need to care about anything for the chips we
#	are interested in (332 and 532 do have more user stuff)
#
set_cpu_type:
	jr	ra

# TODO: the signal return stub will need to go here.
vdso:
#if 0
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
	#To the previously executing code
	#Allowing for the fact rett still points to the svc byte
	addqd	1,0(sp)
	ret	0
#endif
	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0
	.word	0,0,0,0,0,0,0,0

	.data
	.align	2
udata_shadow:
	.long	0
kernel_flag:
	.byte	0
sys_cpu:
	.byte	0
sys_cpu_feat:
	.byte	0
