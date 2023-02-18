#include "../kernel-ns32k.def"


	.globl _platform
	.globl _plt_switchout
	.globl _switchin
	.globl _dofork

#
# Switchout switches out the current process, finds another that is READY,
# possibly the same process, and switches it in.  When a process is
# restarted after calling switchout, it thinks it has just returned
# from switchout().
#
_plt_switchout:
	sprw psr,r0		# Interrupts off
	andw 0xF7FF,r0
	lprw psr,r0

        # save machine state

        # return from either switchout OR dofork, so they must both write
        # U_DATA__U_SP with the following on the stack:
	sprd usp,r1
	# Use enter so we stack fp. We will always exit [] on resume
	enter [r1,r2,r3,r4,r5,r6,r7],0
        movqw 0, tos	# return code set here is ignored, but switchin can
	lprd sp,r0
	movd r0,U_DATA__U_SP(r6)	# this is where the SP is restored in switchin

        # find another process to run (may select this one again)
        bsr _getproc

	movd r0,tos
        bsr _switchin

        # We should never get here
        br _plt_monitor

_switchin:
	sprw psr,r0		# Interrupts off
	andw 0xF7FF,r0
	lprw psr,r0
	enter [],0
	movd 4(fp),r0		# task to switch to
	movd P_TAB__P_UDATA_OFFSET(r0),r5
	# Check udata is consistent wiht pointer
	cmpl U_DATA__U_PTAB(r6),r0
	bne switchinfail

	movd r0,_udata_shadow(pc)		# For IRQ etc use
        movd U_DATA__U_SP(r6),r0
	lprd sp,r0
	movqb P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)

	movqw 0,tos
	movd r0,tos
	jsr _pagemap_switch
	cmpd tos,tos			# Unstack 8 bytes

        # runticks = 0
	movqw 0,_runticks(pc)

        ; restore machine state
	movw tos,r0			# FIXME: can we merge ?
	exit [r1,r2,r3,r4,r5,r6,r7]
	lprd usp,r1

        cmpqb 0,U_DATA__U_ININTERRUPT(r6)
        bne keepoff # in ISR, leave interrupts off
	sprw psr,r0
	orw 0x0800,r0
	lprw psr,r0
keepoff:
        ret 0 # return with interrupts on

switchinfail:
	bsr outr0hex
        addr badswitchmsg,r0
        bsr outstring
	# something went wrong and we didn't switch in what we asked for
        br _plt_monitor

	#
	# Entry:
	# r6 = u_data pointer for parent
	# 4(fp) = child process table entry
	#
	# Exit:
	# Child state is saved, still running as parent
	#
_dofork:
	enter [r1,r2,r3,r4,r5,r6,r7],0
	movd 4(fp),r0			#	child p_tab
	movd P_TAB__P_UDATA_OFFSET(r0),r2	# Child udata

	# copy parent udata in r6 to child udata
	movd r6,r1
	movd 64,r0
	movsd

	movd 4(fp),r0			#	child p_tab
	movw P_TAB__P_PID_OFFSET(r0), tos
	movd P_TAB__P_UDATA_OFFSET(r0), tos

	# Configure the child udata
	movd P_TAB__P_UDATA_OFFSET(r0), tos
	movd r0, tos
	jsr _makeproc
	cmpd tos,tos

	movd tos, r3
	#	Top of child stack
	movd r3,r1
	addd 1024,r1

	#	Stack fake return frame for the child
	#	TODO

	#	Now push a user frame to match switchin/out
	#	TODO


	# Save the built stack base for the child
	movd	r1,U_DATA__U_SP(r2)
	movqw	0,_runticks(pc)
	movd	tos,r0		# Return child pid
	exit	[r1,r2,r3,r4,r5,r6,r7]
	ret	0

badswitchmsg:
	.ascii "_switchin: FAIL"
        .byte 13,10,0

	.align 2

