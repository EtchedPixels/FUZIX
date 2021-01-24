.section .text.set_cpu_type
.align 4
.global set_cpu_type
set_cpu_type:
	movi a2, 0
	movi a3, sys_cpu
	s8i a2, a3, 0
	ret

.comm sys_cpu, 1
.comm sys_cpu_feat, 1

