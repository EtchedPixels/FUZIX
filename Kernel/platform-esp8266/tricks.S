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
dofork_s:
	.string "dofork"
.align 4
dofork:
	movi a2, dofork_s
	j panic

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

