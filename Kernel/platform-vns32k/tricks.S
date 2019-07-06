#include "../kernel-ns32k.def"


	.globl _platform
	.globl _platform_switchout
	.globl _switchin
	.globl _dofork
	.globl _udata_shadow

#
# Switchout switches out the current process, finds another that is READY,
# possibly the same process, and switches it in.  When a process is
# restarted after calling switchout, it thinks it has just returned
# from switchout().
#
_platform_switchout:
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
        br _platform_monitor

_switchin:
	sprw psr,r0		# Interrupts off
	andw 0xF7FF,r0
	lprw psr,r0
	enter [],0
	movd 4(fp),r0		# task to switch to
	movd P_TAB__P_UDATA_OFFSET(r0),r6
	cmpqw 0,P_TAB__P_PAGE_OFFSET(r0)	# swapped or existing process ?
	bne not_swapped

#
#	If you have one udata and swap them then you need to stack switch
#	for the swap in as you'll overwrite the kernel stack. You can use
# 	the IRQ stack but only if you leave IRQs off for a swap in (ugghh)
#

#
#	FIXME: sort IRQ enables
#

#
#	In simple mode the existing process always gets the boot here
#	before we can swap the new one in
#
	movd U_DATA__U_PTAB(r6),r1	# old process
	cmpqw 0,P_TAB__P_PAGE_OFFSET(r1)	# swapped out/dead ?
	beq its_dead_jim		# corpses don't need swapping out

	movd r0,tos
	movd r1,tos
	bsr _swapout
	cmpd tos,tos			# Drop the two stacked arguments

its_dead_jim:
	sprd sp,r1
	addr _irqstack+256,r2		# FIXME save restore r2 ?
	lprd sp,r2
	movd r1,tos
	movd r0,tos
	movd r0,tos
	bsr _swapper
	movd r0,tos
	movd r0,tos
	movqw 1,P_TAB__P_PAGE_OFFSET(r0) # swapped in
	movd tos,r1
	lprd sp,r1

	sprw psr,r1		# Interrupts off
	andw 0xF7FF,r1
	lprw psr,r1

not_swapped:
        # check u_data->u_ptab matches what we wanted
	cmpd U_DATA__U_PTAB(r6),r0
	bne switchinfail

	movqb P_RUNNING,P_TAB__P_STATUS_OFFSET(r0)

        # runticks = 0
	movqw 0,_runticks(pc)

        ; restore machine state
        movd U_DATA__U_SP(r6),r0
	lprd sp,r0
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
        br _platform_monitor

	#
	# this gets exciting on the NS32K because our udata is not in a
	# fixed location except for swap only platforms. That means any
	# udata relative pointers on the stack when we duplicate the kernel
	# stack point to the parent. For the simple case we have a single
	# swapped udata and stack so all is fairly easy. For the other
	# cases we have to return as the parent but set up a fake stack
	# frame for the child to exit the syscall. Simply being careful
	# about pointers doesn't work - the compiler will internally
	# use enter/exit and other stuff.
	#
	# Entry:
	# r6 = u_data pointer for parent
	# 4(fp) = child process table entry
	#
	# Exit:
	# We are running as the child, A5 = u_data pointer of child, on
	# child stack
	#
_dofork:
	#
	# simple edition for swap only
	#
	lprd usp,r1
	enter [r1,r2,r3,r4,r5,r6,r7],0
	movd 4(fp),r0			#	child p_tab
	#
	# in the simple case we only have one udata. In the complex cases
	# we would have to compute the new one and load it into a5 and
	# offset
	#
	movd r6,P_TAB__P_UDATA_OFFSET(r0)
	movw P_TAB__P_PID_OFFSET(r0),tos #	child pid (parent return)
	lprd sp,r0
	movd r0,U_DATA__U_SP(r6)	#	save pointer
	movd U_DATA__U_PTAB(r6),tos	#	parent p_tab

	# FIXME: for the single case with less swaps than processes the
	# out of swap case is one out (we must swap out to swap in so need
	# one swap free after the fork. Right now we deal with this in
	# Config by not setting it that way
	bsr _swapout
	cmpqd 0,tos			#	Discard argument
	cmpqb 0,r0
	bne forked_up

	movd r6,tos			#	U_DATA
	movd 4(fp),tos			#	Process table
	bsr _makeproc
	cmpd tos,tos			#	Cheap way to dump 8 bytes

	movqd 0,r0			#	child, ok
	movqd 0,_runticks(pc)
	exit [r1,r2,r3,r4,r5,r6,r7]
	ret 0
forked_up:
	exit [r1,r2,r3,r4,r5,r6,r7]
	movqd -1,r0			#	parent, failed
	ret 0

badswitchmsg:
	.ascii "_switchin: FAIL"
        .byte 13,10,0

	.align 2

