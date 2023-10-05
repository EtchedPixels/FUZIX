#include "../kernel-ns32k.def"


	.globl	plt_switchout
	.globl	switchin
	.globl	dofork

	.globl	forkout		# For debug
#
# Switchout switches out the current process, finds another that is READY,
# possibly the same process, and switches it in.  When a process is
# restarted after calling switchout, it thinks it has just returned
# from switchout().
#
plt_switchout:
        # save machine state. Ugly as there is a lot if it!
	addi	sp,sp,-52
	sw	ra,(sp)
	sw	s0,4(sp)
	sw	s1,8(sp)
	sw	s2,12(sp)
	sw	s3,16(sp)
	sw	s4,20(sp)
	sw	s5,24(sp)
	sw	s6,28(sp)
	sw	s7,32(sp)
	sw	s8,36(sp)
	sw	s9,40(sp)
	sw	s10,44(sp)
	sw	s11,48(sp)

	jal	__hard_di

	sw	sp,U_DATA__U_SP(tp)	# this is where the SP is restored in switchin

        # find another process to run (may select this one again)
  	call	getproc
        call	switchin

        # We should never get here
        j	plt_monitor

#
#	We never return (we reappear as some saved context. As a result
#	we can be a bit happy about bashing saved registers
#
switchin:
	jal	__hard_di

	# a1 is the task to switch to
	mv	s0,a1
	
	lw	s1, P_TAB__P_UDATA_OFFSET(s0)
	# Check udata is consistent with pointer
	lw	t0,U_DATA__U_PTAB(s1)
	bne	t0,s1,switchinfail

	li	t2,P_RUNNING
	sb	t2,P_TAB__P_STATUS_OFFSET(s0)

	sw	s1, %lo(udata_shadow)(gp)	# For IRQ etc use
        lw	sp,U_DATA__U_SP(s1)		# Recover kernel stack pointer

	#	A1 is still the task
	mv	a2,zero
	call	pagemap_switch

        # runticks = 0
	la	t0,runticks
	sw	zero,0(t0)

        lb	t0,U_DATA__U_ININTERRUPT(tp)
        bne	t0,zero,keepoff # in ISR, leave interrupts off
	jal	__hard_ei
keepoff:
	mv	a1,zero
 	# restore machine state from the recovered kstack
	lw	ra,(sp)
	lw	s0,4(sp)
	lw	s1,8(sp)
	lw	s2,12(sp)
	lw	s3,16(sp)
	lw	s4,20(sp)
	lw	s5,24(sp)
	lw	s6,28(sp)
	lw	s7,32(sp)
	lw	s8,36(sp)
	lw	s9,40(sp)
	lw	s10,44(sp)
	lw	s11,48(sp)
	addi	sp,sp,52
	jr	ra

switchinfail:
        la	a1,badswitchmsg
        call	outstring
	# something went wrong and we didn't switch in what we asked for
        j	plt_monitor

	#
	# Entry:
	# tp = u_data pointer for parent
	# a1 = child process table entry
	#
	# Exit:
	# Child state is saved, still running as parent
	#
dofork:

	lw	t0,P_TAB__P_UDATA_OFFSET(a1)

	# copy parent udata in r6 to child udata
	mv	t1,tp
	li	t2,UBLOCK_SIZE/4
save_ud:
	lw	t3,0(t1)
	sw	t3,0(t0)
	addi	t1,t1,1
	addi	t0,t0,1
	addi	t2,t2,-1
	bne	t2,zero,save_ud

	#	Save A1,RA somewhere

	addi	sp,sp,-8
	sw	a1,4(sp)
	sw	ra,(sp)

	# Configure the child udata
	lw	a2,P_TAB__P_UDATA_OFFSET(a1)
	call	makeproc

	lw	a1,4(sp)
	lw	ra,(sp)
	addi	sp,sp,8

	#	Top of the child stack to fill in
	lw	a2,P_TAB__P_UDATA_OFFSET(a1)
	addi	t0,a2,UBLOCK_SIZE
	addi	t1,tp,UBLOCK_SIZE

	lw	t2,-4(t1)		# tp
	sw	t2,-4(t0)
	lw	t2,-8(t1)		# gp
	sw	t2,-8(t0)
	lw	t2,-12(t1)		# ra
	sw	t2,-12(t0)
	la	t2,forkout
	sw	t2,-16(t0)		# and our return

	# TODO: save register frame here

	addi	t0,t0,-16		# SP to use for child
					# fork unwrap

	#	The child stack is now a valid switchin frame
	#	which will return to forkout, and then it will
	#	rett back to user space

	# Save the built stack base for the child
	sw	t0,U_DATA__U_SP(a2)
	sw	zero,%lo(runticks)(gp)
	# Get child PID
	lhu	a1,P_TAB__P_PID_OFFSET(a1)
	jr	ra

forkout:
	lw	ra,8(sp)
	lw	gp,4(sp)
	lw	tp,0(sp)
	sb	zero,%lo(kernel_flag)(gp)
	sb	zero,U_DATA__U_INSYS(tp)
	jr	ra

badswitchmsg:
	.ascii "_switchin: FAIL"
        .byte 13,10,0

	.align 2
