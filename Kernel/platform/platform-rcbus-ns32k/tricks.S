#include "../../cpu-ns32k/kernel-ns32k.def"


	.globl	_plt_switchout
	.globl	_switchin
	.globl	_dofork

	.globl	forkout		# For debug
#
# Switchout switches out the current process, finds another that is READY,
# possibly the same process, and switches it in.  When a process is
# restarted after calling switchout, it thinks it has just returned
# from switchout().
#
_plt_switchout:
	sprw	psr,r0		# Interrupts off
	andw	0xF7FF,r0
	lprw	psr,r0

        # save machine state

        # return from either switchout OR dofork, so they must both write
        # U_DATA__U_SP with the following on the stack:
	sprd	usp,r1
	# Use enter so we stack fp. We will always exit [] on resume
	enter	[r1,r2,r3,r4,r5,r7],0
	sprd	fp,r0
	movd	r0,tos			# Save fp
	sprd	sp,r0
	movd	r0,U_DATA__U_SP(r6)	# this is where the SP is restored in switchin

        # find another process to run (may select this one again)
        bsr	_getproc(pc)

	movd r0,tos
        bsr	_switchin(pc)

        # We should never get here
        br	_plt_monitor

_switchin:
	sprw	psr,r0		# Interrupts off
	andw	0xF7FF,r0
	lprw	psr,r0
	enter	[],0
	movd	8(fp),r0	# task to switch to
	movd	P_TAB__P_UDATA_OFFSET(r0),r6
	# Check udata is consistent with pointer
	cmpd	U_DATA__U_PTAB(r6),r0
	bne	switchinfail

	movqb	P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)
	movd	r6,_udata_shadow(pc)	# For IRQ etc use
        movd	U_DATA__U_SP(r6),r1	# Recover kernel stack pointer
	lprd	sp,r1

	movqd	0,tos
	movd	r0,tos
	bsr	_pagemap_switch(pc)
	adjspb	-8			# Unstack 8 bytes

        # runticks = 0
	movqw	0,_runticks(pc)

 	# restore machine state from the recovered kstack
	movd	tos,r1			# Top of the stack is the saved fp
	lprd	fp,r1
	exit	[r1,r2,r3,r4,r5,r7]	# That allows us to get the registers
	lprd	usp,r1			# Restore user SP saved in R1

        cmpqb	0,U_DATA__U_ININTERRUPT(r6)
        bne	keepoff # in ISR, leave interrupts off
	sprw	psr,r0
	orw	0x0800,r0
	lprw	psr,r0
keepoff:
	movqw	0,r0			# fork return never comes this route
        ret	0			# return with interrupts on

switchinfail:
	bsr	outr0hex
        addr	badswitchmsg,r0
        bsr	outstring
	# something went wrong and we didn't switch in what we asked for
        br	_plt_monitor

	#
	# Entry:
	# r6 = u_data pointer for parent
	# stack = child process table entry
	#
	# Exit:
	# Child state is saved, still running as parent
	#
_dofork:
	enter	[r1,r2,r3,r4,r5,r7],0
	movd	8(fp),r0			#	child p_tab
	movd	P_TAB__P_UDATA_OFFSET(r0),r2	# Child udata

	# copy parent udata in r6 to child udata
	movd	r6,r1
	movd	64,r0
	movsd


	movd	8(fp),r0		#	child p_tab
	movw	P_TAB__P_PID_OFFSET(r0), tos
	movd	P_TAB__P_UDATA_OFFSET(r0), tos

	# Configure the child udata
	movd	P_TAB__P_UDATA_OFFSET(r0), tos
	movd	r0, tos
	bsr	_makeproc(pc)
	adjspb	-8		# Adjsp is subtract based !

	movd	tos, r3
	#	Top of the child stack to fill in
	movd	r3,r1
	addd	1024-20,r1

	#	R6 holds our udata, find the stack top from it
	movd	r6,r4
	#	Pointer for copying from the parent to child
	addd	1024-16,r4

	# The parent entered with enter [r6]
	# Copy the saved r6 and FP over
	movd	0(r4),4(r1)
	movd	4(r4),8(r1)
	# Copy the trap frame over
	movd	8(r4),12(r1)
	movd	12(r4),16(r1)
	addqd	1,12(r1)	# Still points at the SVC
	addr	forkout(pc),0(r1)

	#	Now build a switchin frame under the stack
	#
	#	It needs to look like this
	#	
	#	[Trap frame]
	#	R7/R5-R1 	# R1 saved USP
	#	Old FP
	#	FP
	movd	r1,r0		# Old FP
	addd	8,r0
	subd	32,r1
	movd	r0,28(r1)	# Old FP
	# The registers (don't care)
	sprd	usp,24(r1)	# R1: Save USP into frame
	movd	0,20(r1)	# R2
	movd	0,16(r1)	# R3
	movd	0,12(r1)	# R4
	movd	0,8(r1)		# R5
	movd	0xFEDCBA98,4(r1)	# R7
	# 	Frame Pointer
	movd	r1,r0
	addd	28 ,r0		# Need to figure out
	movd	r0,0(r1)

	#	The child stack is now a valid switchin frame
	#	which will return to forkout, and then it will
	#	rett back to user space

	# Save the built stack base for the child
	movd	r1,U_DATA__U_SP(r3)
	movqw	0,_runticks(pc)
	movzwd	tos,r0		# Return child pid
	exit	[r1,r2,r3,r4,r5,r7]
	ret	0

forkout:
	# FIXME: needs to clear in sys etc
	# Check if similar bug on 68000
	movd	_udata_shadow(pc),r1
	movqd	0,r0
	movb	r0,_kernel_flag(pc)
	movb	r0,U_DATA__U_INSYS(r1)
	movd	r0,r1
	movd	r1,r2
	movd	r2,r3
	movd	r3,r4
	movd	r4,r5
	movd	r5,r7
	exit	[r6]
	rett	0

badswitchmsg:
	.ascii "_switchin: FAIL"
        .byte 13,10,0

	.align 2
