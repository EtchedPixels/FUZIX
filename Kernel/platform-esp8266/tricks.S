#include "kernel-lx106.def"

.global udata
.comm udata, 512

.global platform_switchout
.text
switchout_s:
	.string "platform_switchout"
.align 4
platform_switchout:
	movi a2, switchout_s
	j panic

.global switchin
.text
switchin_s:
	.string "platform_switchin"
.align 4
switchin:
	movi a2, switchin_s
	j panic

.global dofork
.text
.align 4
# a2 = ptab of the current process
dofork:
	/* Save the callee-saved registers onto the stack in the same order
	 * that switchout will do it, so switchin will restore it correctly. */

	addi sp, sp, -8*4
	s32i a12, sp, 0*4
	s32i a13, sp, 1*4
	s32i a14, sp, 2*4
	s32i a15, sp, 3*4
	rsr.sar a3
	s32i a3, sp, 4*4
	l32i a3, a2, P_TAB__P_PID_OFFSET
	s32i a3, sp, 5*4
	movi a3, udata
	s32i sp, a3, U_DATA__U_SP_OFFSET

	/* Save the current process to disk. */

	s32i a2, sp, 6*4
	l32i a2, a3, U_DATA__U_PTAB_OFFSET
	call0 swapout
	l32i a2, sp, 6*4

	/* We are now going to become the child; create a process structure
	 * for us. */

	movi a3, udata
	call0 makeproc
	
	/* We are now ready to return. */

	movi a2, 0
	movi a3, runticks
	s32i a2, a3, 0

	l32i a12, sp, 0*4
	l32i a13, sp, 1*4
	l32i a14, sp, 2*4
	l32i a15, sp, 3*4
	l32i a3, sp, 4*4
	rsr.sar a3
	l32i a2, sp, 5*4

	addi sp, sp, 8*4
	ret

.global platform_idle
.text
.align 4
platform_idle:
	ret

.global program_vectors
.text
.align 4
program_vectors:
	ret

.global stub_unix_syscall
.text
.align 4
# a2 = exception frame structure
# a3 = cause
stub_unix_syscall:
	s32i sp, a2, 12
	movi sp, udata + UBLOCK_SIZE

	call0 unix_syscall

	l32i sp, a2, 12
	ret

