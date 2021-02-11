#include "kernel-lx106.def"

.section .text.set_cpu_type
.align 4
.global set_cpu_type
set_cpu_type:
	movi a2, 11
	movi a3, sys_cpu
	s8i a2, a3, 0
	ret

.comm sys_cpu, 1
.comm sys_cpu_feat, 1

.section .text.platform_doexec
.align 4
.global platform_doexec
# a2 = start address
# a3 = new stack pointer
platform_doexec:
	movi a4, 32
	wsr.ps a4			# set user mode (and enable interrupts)
	mov sp, a3
	isync
	jx a2

