.section .text.__hard_ei
.align 4
.global __hard_ei
__hard_ei:
	movi a2, -1
	wsr.intenable a2
	ret

.section .text.__hard_di
.align 4
.global __hard_di
__hard_di:
	rsr.intenable a2
	movi a3, 0
	wsr.intenable a3
	ret

.section .text.__hard_irqrestore
.align 4
.global __hard_irqrestore
__hard_irqrestore:
	wsr.intenable a2
	ret

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

