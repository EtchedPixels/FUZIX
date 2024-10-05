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

.section .text.plt_doexec
.align 4
.global plt_doexec
# a2 = start address
# a3 = new stack pointer
plt_doexec:
	movi a4, 32
	wsr.ps a4			# set user mode (and enable interrupts)
	mov.n sp, a3
	isync
	jx a2

.section .text.vectors
.align 16
.global platform_vectors
platform_vectors:			# Space for vector handler literals
	nop
	.literal_position		# Tell the assembler where to put them (max 3)
.align 16
debug:					# Debug exception at 0x10
	waiti 2
	j debug
.align 16
nmi:					# NMI trap at 0x20
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

					# On a reset the vector is reset
					# before read so doesn't come here

.section .text.handlers
.align 4

	.literal_position

.global vectortable
vectortable:
	.word  fault			# Illegal instruction	 	0
	.word  syscall			# System call			1
	.word  fault			# Instruction fetch error	2
	.word  fault			# Load/store error		3
	.word  interrupt		# Level 1 interrupt		4
	.word  fault			# Window trap			5
	.word  fault			# Divide by zero		6
	.word  fault			#				7
	.word  fault			# Privilege trap		8
	.word  fault			# Load Store alignment		9
	.word  fault			#				10
	.word  fault			#				11
	.word  fault			#				12
	.word  fault			#				13
	.word  fault			#				14
	.word  fault			#				15
	.word  fault			#				16
	.word  fault			#				17
	.word  fault			#				18
	.word  fault			#				19
	.word  fault			#				20
	.word  fault			#				21
	.word  fault			#				22
	.word  fault			#				23
	.word  fault			#				24
	.word  fault			#				25
	.word  fault			#				26
	.word  fault			#				27
	.word  fault			# Invalid address (I)		28
	.word  fault			# Invalid address (D)		29
	.word  fault			#				30
	.word  fault			#				31
	.word  fault			#				32
	.word  fault			#				33
	.word  fault			#				34
	.word  fault			#				35
	.word  fault			#				36
	.word  fault			#				37
	.word  fault			#				38
	.word  fault			#				39
	.word  fault			#				40
	.word  fault			#				41
	.word  fault			#				42
	.word  fault			#				43
	.word  fault			#				44
	.word  fault			#				45
	.word  fault			#				46
	.word  fault			#				47
	.word  fault			#				48
	.word  fault			#				49
	.word  fault			#				50
	.word  fault			#				51
	.word  fault			#				52
	.word  fault			#				53
	.word  fault			#				54
	.word  fault			#				55
	.word  fault			#				56
	.word  fault			#				57
	.word  fault			#				58
	.word  fault			#				59
	.word  fault			#				60
	.word  fault			#				61
	.word  fault			#				62
	.word  fault			#				63



exception:
	addi sp, sp, -8
	# Stack the general registers we want saved
	s32i.n a12, sp, 0
	s32i.n a13, sp, 4
	movi a13, vectortable
	rsr.exccause a12
	addx4 a13, a12, a13
	l32i.n a13,a13,0
	jx a13
ignored:
	l32i.n a12, sp, 0
	l32i.n a13, sp, 4
	addi sp, sp, 8
	rfe

syscall:
	movi a12, udata
	movi a7, 1
	s8i a7, a12, U_DATA__U_INSYS
	s8i a6, a12, U_DATA__U_CALLNO
	s32i.n a2, a12, U_DATA__U_ARGN
	s32i.n a3, a12, U_DATA__U_ARGN1
	s32i.n a4, a12, U_DATA__U_ARGN2
	s32i.n a5, a12, U_DATA__U_ARGN3
	s32i.n sp, a12, U_DATA__U_SYSCALL_SP
	movi sp, udata + UBLOCK_SIZE - 32
	s32i.n a0, sp, 0
	s32i.n a14, sp, 4
	s32i.n a15, sp, 8
	rsr.epc1 a12
	s32i.n a12, sp, 12
	movi a12, 0x2F
	xsr.ps a12
	s32i.n a12, sp, 16
	rsync
	# TODO set insys properly and enable ints for syscall section
	call0 unix_syscall
	l32i.n a12, sp, 16
	wsr.ps a12
	rsync
	l32i.n a12, sp, 12
	addi a12,a12, 3		# Skip the syscall
	wsr.epc1 a12
	movi a12, udata
	l8ui a4, a12, U_DATA__U_CURSIG
	l16ui a3, a12, U_DATA__U_ERROR
	l32i.n a2, a12, U_DATA__U_RETVAL
	l32i.n a15, sp, 8
	l32i.n a14, sp, 4
	l32i.n a0, sp, 0
	movi a12, udata
	movi a7, 0
	s8i a7, a12, U_DATA__U_INSYS
	l32i.n sp, a12, U_DATA__U_SYSCALL_SP
	bnez a4, deliver_sig
sigout:
	l32i.n a13, sp, 4
	l32i.n a12, sp, 0
	addi sp, sp, 8
	rsync
	rfe

deliver_sig:
	addi sp, sp, -24	# keep aligned (8 already pushed)
	s32i.n a2, sp, 0		# Save syscall return values
	s32i.n a3, sp, 4
	s32i.n a14, sp, 8
	s32i.n a15, sp, 12
	s32i.n a0, sp, 16
	rsr.epc1 a14
	s32i.n a14, sp, 20
	addx4 a2, a4, a12	# Signal vector time
	l32i.n a5, a2, U_DATA__U_SIGVEC
	beqz a5, no_signal
	movi a14, 0
	s32i.n a14, a2, U_DATA__U_SIGVEC
	s8i  a14, a12, U_DATA__U_CURSIG
	#
	#	A4 is the signal, A5 is the vector
	#
	mov.n a2,a4		# We want it in arg1
	mov.n a3,a1		# second arg is the stack frame
	movi a4,0x20		# User mode and status
	wsr.ps a4		# Into user mode
	rsync
	callx0 a5
	#
	#	Now clean back up and return from the syscall. Note that we
	#	may never reach this point but that is fine.
	#
	rsil a14, 15			# Interrupts back off
	rsync
no_signal:
	l32i.n a14, sp, 20
	wsr.epc1 a14
	l32i.n a0, sp, 16
	l32i.n a15, sp, 12
	l32i.n a14, sp, 8
	l32i.n a3, sp, 4
	l32i.n a2, sp, 0
	addi sp, sp, 24
	j sigout
#
#	We take an exception on whatever stack we are using.
#
fault:
	# Our caller saved a12 and a13 save the rest for the helper
	addi sp, sp, -72		# 8 already pushed so this keeps us aligned
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
	mov.n a3, a12			# Cause as second argument
	s32i.n a12, sp, 64
	rsr.excvaddr a12
	s32i.n a12, sp, 68
	# Pass the frame pointer as a C argument
	mov.n a2,a1
	call0 exception_handler
	# Does the exception handler want to force a synchronous signal ?
	beqz a2, no_action
	movi a12, udata
	addx4 a13, a2, a12
	l32i.n a12, a13, U_DATA__U_SIGVEC
	movi a3, 0
	s32i.n a3, a13, U_DATA__U_SIGVEC
	movi a3, 0x20
	wsr.ps a3
	rsync
	mov.n a3, a1			# Stack frame pointer
	callx0 a13			# Deliver the signal (may or may not return)
	rsil a12, 15			# Interrupts back off
no_action:
	# Only restore the state the compiler damaged
	l32i.n a12, sp, 56
	wsr.sar a12
	l32i.n a12, sp, 60
	wsr.epc1 a12
	l32i.n a0, sp, 0
	l32i.n a2, sp, 4
	l32i.n a3, sp, 8
	l32i.n a4, sp, 12
	l32i.n a5, sp, 16
	l32i.n a6, sp, 20
	l32i.n a7, sp, 24
	l32i.n a14, sp, 48
	l32i.n a15, sp, 42
	l32i.n a12, sp, 72
	l32i.n a13, sp, 76
	addi sp, sp, 80
	rsync
	rfe
#
#	Interrupt. This is the trickiest one as we need to ensure nothing
#	gets mashed, but save and restore the minimum for efficiency. We also
# 	have to deal with the interrupt stack and eventually pre-emption and
#	signals
#
interrupt:
	addi sp, sp, -44
	# A12/A13 are saved. We need to save A14/A15/A0 SAR and EPC1
	s32i.n a14, sp, 0
	s32i.n a15, sp, 4
	s32i.n a0, sp, 8
	rsr.sar a12
	s32i.n a12, sp, 12
	rsr.epc1 a12
	s32i.n a12, sp, 16
	# Save the argument registers
	s32i.n a2, sp, 20
	s32i.n a3, sp, 24
	s32i.n a4, sp, 28
	s32i.n a5, sp, 32
	s32i.n a6, sp, 36
	s32i.n a7, sp, 40
	rsr.interrupt a2
	# Stack switch - we keep the minimum on istack for preemption
	# reasons
	mov.n a2,a1
	movi a1, istack_end - 4
	s32i.n a2, a1, 0			# Save it in the word at the top
	movi a3, udata
	movi a2, 1
	s8i a2, a3, U_DATA__U_ININTERRUPT
	rsr.interrupt a2
	rsr.intenable a3
	and a2,a2,a3
	movi a3,0x3FFF
	and a2,a2,a3
	call0 interrupt_handler
	# Restore the state
	movi a2, istack_end - 4
	l32i.n a1, a2, 0
	movi a12, udata
	l8ui a2, a12, U_DATA__U_INSYS
	beqz a2, restore
	movi a2, need_resched
	l8ui a2, a2, 0
	bnez a2, preemption
	# TODO signals
	l8ui a2, a12, U_DATA__U_CURSIG
	beqz a2, restore
	movi a3,0
	s8i a3, a2, U_DATA__U_CURSIG
	addx4 a4, a2, a12
	l32i.n a13, a4, U_DATA__U_SIGVEC
	beqz a13, restore
	s32i.n a3, a4, U_DATA__U_SIGVEC
	# We are on the user stack, we just need to flip a few flags around
	# Interrupts are still off
	s8i a3, a12, U_DATA__U_ININTERRUPT
	# Signal code is already in a2
	# Get the frame base into a3
	mov.n a3, a1
	# Interrupts back on
	movi a14, 0x20
	xsr.ps a14
	rsync
	# Call the handler (which may or may not return)
	callx0 a13
	# If it returns we restore our irq state and stack frame
	wsr.ps a14
	rsync
restore:
	movi a3, udata
	movi a2, 0
	s8i a2, a3, U_DATA__U_ININTERRUPT
	# Recover register state
	l32i.n a7, sp, 40
	l32i.n a6, sp, 36
	l32i.n a5, sp, 32
	l32i.n a4, sp, 28
	l32i.n a3, sp, 24
	l32i.n a2, sp, 20
	l32i.n a12, sp, 16
	wsr.epc1 a12
	l32i.n a12, sp, 12
	wsr.sar a12
	l32i.n a0, sp, 8
	l32i.n a15, sp, 4
	l32i.n a14, sp, 0
	l32i.n a13, sp, 48
	l32i.n a12, sp, 44
	addi sp, sp, 52
	rsync
	rfe

preemption:
	# All our state is on the user stack and mapped/saved, so this is a doddle
	# A12 still points to udata
	s32i.n a1, a12, U_DATA__U_SYSCALL_SP
	movi a12, udata + UBLOCK_SIZE
	call0 switchout
	# switching will handle all the interrupt state for us. Note that we
	# may never return and also that the IRQ exit path is actually long
	# gone - but we saved epc1 so all is good.
	j restore
