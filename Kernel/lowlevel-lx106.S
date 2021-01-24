.section .text.__hard_ei
.align 4
.global __hard_ei
# Turns on interrupts.
__hard_ei:
	rsil a2, 0
	ret

.section .text.__hard_di
.align 4
.global __hard_di
# Turns off interrupts and returns the old status in a2.
__hard_di:
	rsil a2, 15
	ret

.section .text.__hard_irqrestore
.align 4
.global __hard_irqrestore
# Sets the interrupt status to a2.
__hard_irqrestore:
	wsr a2, ps
	isync
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

