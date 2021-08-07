#include "kernel-esp8266.def"

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

.section .text.vectors
.align 16
.global platform_vectors
confused:				# Nothing should hit this
	movi a2, system_restart
	jx a2
.align 16
debug:					# Debug exception at 0x20
	waiti 2
	j debug
.align 16
nmi:					# NMI trap at 0x30
	rfi 3
.align 16
kexc:					# Kernel mode trap at 0x30
	break 1,0
	j kexc
.align 16
unused:					# Nothing at 0x40
	j unused
.align 16
uexc:					# User exception at 0x50
	j exception			# User exception (pretty much everything for us)
.align 16
unused2:
	j unused2			# Nothing at 0x60
.align 16
doublefault:				# Double fault at 0x70
	break 1,4
	j doublefault
.align 16
#reset:
#	j 0x400000a4

.section .data
.align 4
vectortable:
	.long  fault			# Illegal instruction	 	0
	.long  syscall			# System call			1
	.long  fault			# Instruction fetch error	2
	.long  fault			# Load/store error		3
	.long  interrupt		# Level 1 interrupt		4
	.long  fault			# Window trap			5
	.long  fault			# Divide by zero		6
	.long  fault			#				7
	.long  fault			# Privilege trap		8
	.long  fault			# Load Store alignment		9
	.long  fault			#				10
	.long  fault			#				11
	.long  fault			#				12
	.long  fault			#				13
	.long  fault			#				14
	.long  fault			#				15
	.long  fault			#				16
	.long  fault			#				17
	.long  fault			#				18
	.long  fault			#				19
	.long  fault			#				20
	.long  fault			#				21
	.long  fault			#				22
	.long  fault			#				23
	.long  fault			#				24
	.long  fault			#				25
	.long  fault			#				26
	.long  fault			#				27
	.long  fault			# Invalid address (I)		28
	.long  fault			# Invalid address (D)		29
	.long  fault			#				30
	.long  fault			#				31
	.long  fault			#				32
	.long  fault			#				33
	.long  fault			#				34
	.long  fault			#				35
	.long  fault			#				36
	.long  fault			#				37
	.long  fault			#				38
	.long  fault			#				39
	.long  fault			#				40
	.long  fault			#				41
	.long  fault			#				42
	.long  fault			#				43
	.long  fault			#				44
	.long  fault			#				45
	.long  fault			#				46
	.long  fault			#				47
	.long  fault			#				48
	.long  fault			#				49
	.long  fault			#				50
	.long  fault			#				51
	.long  fault			#				52
	.long  fault			#				53
	.long  fault			#				54
	.long  fault			#				55
	.long  fault			#				56
	.long  fault			#				57
	.long  fault			#				58
	.long  fault			#				59
	.long  fault			#				60
	.long  fault			#				61
	.long  fault			#				62
	.long  fault			#				63


.section .text.handlers
.align 4

exception:
	addi sp, sp, -8
	# Stack the general registers we want saved
	s32i.n a12, sp, 0
	s32i.n a13, sp, 4
	movi a13, vectortable
	rsr.exccause a12
	addx4 a13, a12, a13
	jx a13
ignored:
	l32i.n a12, sp, 0
	l32i.n a13, sp, 4
	addi sp, sp, 8
	rfe

syscall:
	movi a12, udata
	s8i a2, a12, U_DATA__U_CALLNO
	s32i.n a3, a12, U_DATA__U_ARGN
	s32i.n a4, a12, U_DATA__U_ARGN1
	s32i.n a5, a12, U_DATA__U_ARGN2
	s32i.n a6, a12, U_DATA__U_ARGN3
	s32i.n sp, a12, U_DATA__U_SYSCALL_SP
	movi sp, udata + UBLOCK_SIZE - 20
	s32i.n a0, sp, 0
	s32i.n a14, sp, 4
	s32i.n a15, sp, 8
	rsr.epc1 a12
	s32i a12, sp, 12
	movi a12, 0x2F
	xsr.ps a12
	s32i.n a12, sp, 16
	rsync
	call0 unix_syscall
	l32i.n a12, sp, 16
	wsr.ps a12
	rsync
	l32i.n a12, sp, 12
	wsr.epc1 a12
	l32i.n a15, sp, 8
	l32i.n a14, sp, 4
	l32i.n a0, sp, 0
	movi a12, udata
	l32i.n sp, a12, U_DATA__U_SYSCALL_SP
	l32i.n a13, sp, 4
	l32i.n a12, sp, 0
	addi sp, sp, 8
	rfe

#
#	We take an exception on whatever stack we are using.
#
fault:
	# Our caller saved a12 and a13 save the rest for the helper
	addi sp, sp, -72
	s32i.n a0, sp, 0
	s32i.n a2, sp, 4
	s32i.n a3, sp, 8
	s32i.n a4, sp, 12
	s32i.n a5, sp, 16
	s32i.n a6, sp, 20
	s32i.n a7, sp, 24
	s32i.n a8, sp, 28
	s32i.n a9, sp, 32
	s32i.n a10, sp, 36
	s32i.n a11, sp, 40
	s32i.n a14, sp, 48
	s32i.n a15, sp, 52
	rsr.sar a12
	s32i.n a12, sp, 56
	rsr.epc1 a12
	s32i.n a12, sp, 60
	rsr.exccause a12
	s32i.n a12, sp, 64
	rsr.excvaddr a12
	s32i.n a12, sp, 68
	call0 exception_handler
	# Only restore the state the compiler damaged
	l32i.n a12, sp, 56
	wsr.sar a12
	l32i.n a12, sp, 60
	wsr.epc1 a12
	l32i.n a0, sp, 0
	l32i.n a14, sp, 48
	l32i.n a15, sp, 42
	addi sp, sp, 72
	l32i.n a13, sp, 4
	l32i.n a12, sp, 0
	addi sp, sp, 8
	rfe
#
#	Interrupt. This is the trickiest one as we need to ensure nothing
#	get mashed, but save and restore the minimum for efficiency. We also
# 	have to deal with the interrupt stack and eventually pre-emption and
#	signals
#
interrupt:
	addi sp, sp, -24
	# A12/A13 are saved. We need to save A14/A15/A0 SAR and EPC1
	s32i.n a14, sp, 0
	s32i.n a15, sp, 4
	s32i.n a0, sp, 8
	rsr.sar a12
	s32i.n a12, sp, 12
	rsr.epc1 a12
	s32i.n a12, sp, 16
	# We choose to save A2 so we can pass the interrupt in
	s32i.n a2, sp, 20
	rsr.interrupt a2
	# Stack switch - we keep the minimum on istack for preemption
	# reasons
	movi a12, udata
	s32i.n a1, a2, U_DATA__U_SP
	movi a12, istack_top
	call0 interrupt_handler
	# Restore the state
	movi a12, udata
	l32i.n a1, a12, U_DATA__U_SP
	l8ui a2, a12, U_DATA__U_INSYS
	beqz a2, restore
	movi a2, need_resched
	l8ui a2, a2, 0
	bnez a2, preemption
	# TODO signals
restore:
	# Recover register state
	l32i.n a2, sp, 20
	l32i.n a12, sp, 16
	wsr.epc1 a12
	l32i.n a12, sp, 12
	wsr.sar a12
	l32i.n a0, sp, 8
	l32i.n a15, sp, 4
	l32i.n a14, sp, 0
	addi sp, sp, 24
	l32i.n a13, sp, 4
	l32i.n a12, sp, 0
	addi sp, sp, 8
	rfe

preemption:
	# All our state is on the user stack and mapped/saved, so this is a doddle
	# A12 still points to udata
	s32i.n a1, a12, U_DATA__U_SYSCALL_SP
	movi a12, udata + UBLOCK_SIZE
	call0 switchout
	# switching will handle all the interrupt state for us. Note that we
	# may never return and also that the IRQ exit path is actually long
	# done - but we saved epc1 so all is good.
	j restore
