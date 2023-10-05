#include "../../cpu-riscv32/kernel-riscv32.def"
/*
 *	Lots left to fill in
 */
	.globl	plt_reboot
	.globl	init_early
	.globl	init_hardware
	.globl	program_vectors
	.globl	outchar
	.globl	plt_monitor
	.globl	udata_block
	.globl	udata_shadow
	.globl  __hard_di
	.globl  __hard_ei
	.globl	__hard_irqrestore

	.globl	syscall_handler

__hard_di:
	csrr	a1, mie
	andi	t0,a1,0x555	# Mask except for our bits
	csrw	mie,t0
	csrw	sie,t0
	csrw	uie,t0
	jr	ra

__hard_ei:
	csrr	t0, mie
	# It's a 12 bit register - call it FFFFFAAA so the assembler
	# knows the sign extend is ok
	ori	t0,a1,0xFFFFFAAA	# All the bits we care about
	csrw	mie,t0
	csrw	sie,t0
	csrw	uie,t0
	jr	ra

__hard_irqrestore:
	csrw	mie,a1
	csrw	sie,a1
	csrw	uie,a1
	jr	ra

plt_reboot:
plt_monitor:
	jal	__hard_di
	j	plt_reboot

init_early:
	la	t0,udata_block
	la	t1,udata_shadow
	sw	t0,(t1)
	jr	ra
#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now) TODO
	li	t0,384
	la	t1,ramsize
	sh	t0,(t1)
	addi	t0,t0,-64
	la	t1,procmem
	sh	t0,(t1)
# Set up vectors
	la	t0,do_exception
	csrw	mtvec,t0
# TODO timer enable etc
	jr	ra

#
#	Nothing to do - all set up once at boot
#
_program_vectors:
	jr	ra

#
#	We do no banking so we need to do nothing here.
#
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	jr	ra

# outchar: Wait for UART TX idle, then print the char
# It's a virtual uart that never overflows but a real one
# would need to check the ready bits
outchar:
	la	t2,0x60000000
	sb	t0,(t2)
	jr	ra

#
#	Exceptions come here. As we don't have multiple modes on our sinple
#	reference everything comes here
#
do_exception:
	.option push
	.option norelax
	csrw	mscratch,t0
	la	t0,udata_shadow
	.option pop
	lw	t0,(t0)
	lw	t0,U_DATA__U_INSYS(t0)
	bne	t0,zero,kirq_handler
via_kstack:
	addi	t0,t0,UBLOCK_SIZE - 16
	j	irq_handler
kirq_handler:
	addi	t0,sp,-16
irq_handler:
	#	Must match dofork() - TODO FIXME
	sw	sp,12(t0)
	sw	ra,8(t0)
	sw	gp,4(t0)
	sw	tp,0(t0)
	addi	sp,t0,0

	.option push
	.option norelax
	la	gp,__global_pointer$
	.option pop
	# tp/sp/gp now set up

	# t0 is usable
	csrr	t0,mcause
	# TODO: test for syscall 9 or 12 ?
	addi	t0,t0,-9
	beq	t0, zero, syscall_handler

	# Exception or IRQ from user. Save the whole context we need
	addi	sp,sp,-64	# Alignment rule
	sw	a7,52(sp)
	sw	a6,48(sp)
	sw	a5,44(sp)
	sw	a4,40(sp)
	sw	a3,36(sp)
	sw	a2,32(sp)
	sw	a1,28(sp)
	sw	a0,24(sp)
	sw	t6,20(sp)
	sw	t5,16(sp)
	sw	t4,12(sp)
	sw	t3,8(sp)
	sw	t2,4(sp)
	sw	t1,0(sp)
	csrr	a0,mepc
	csrr	a1,mcause
	csrr	a2,mtval
	la	tp,udata_shadow
	jal	exception
	#	TODO - interrupt signal return path
	lw	t1,0(sp)
	lw	t2,4(sp)
	lw	t3,8(sp)
	lw	t4,12(sp)
	lw	t5,16(sp)
	lw	t6,20(sp)
	lw	a0,24(sp)
	lw	a1,28(sp)
	lw	a2,32(sp)
	lw	a3,40(sp)
	lw	a5,44(sp)
	lw	a6,48(sp)
	lw	a7,52(sp)
	lw	t0,U_DATA__U_CURSIG(tp)
	bne	t0,zero, irq_via_sig
	# 8 byte gap for alignment from initial entry
	lw	tp,64(sp)
	lw	gp,68(sp)
	lw	ra,72(sp)
	lw	sp,76(sp)
	csrr	t0,mscratch
	mret

irq_via_sig:
	addi	sp,sp,64		# Adjust stack to expected remaining block
	j	return_via_signal

syscall_handler:
	# Interrupts are off due to the trap, and we have set up sp/gp/tp
	sb	a7,U_DATA__U_CALLNO(tp)
	li	t0,1
	sb	t0,U_DATA__U_INSYS(tp)
	sw	a0,U_DATA__U_ARGN(tp)
	sw	a1,U_DATA__U_ARGN1(tp)
	sw	a2,U_DATA__U_ARGN2(tp)
	sw	a3,U_DATA__U_ARGN3(tp)
	la	t1,kernel_flag
	sb	t0,(t1)
	addi	sp,sp,-16
	csrr	t0,mepc
	sw	t0,(sp)
	csrr	t0,mscratch
	sw	t0,4(sp)
	csrr	t0,mpie
	sw	t0,8(sp)
	jal	__hard_ei
	jal	unix_syscall
	jal	__hard_di
	lw	t0,0(sp)
	csrw	mepc,t0

	sb	zero,U_DATA__U_INSYS(tp)
	# kernel flag needs to die TODO
	la	t0,kernel_flag
	sb	zero,(t0)
	lh	a0,U_DATA__U_ERROR(tp)
	bne	a0,zero,sysc_err
	lw	a1,U_DATA__U_RETVAL(tp)
sysc_sig:
	lw	t0,U_DATA__U_CURSIG(tp)
	bne	t1,zero,return_via_signal
sysc_ret:
	lw	t0,0(sp)
	csrw	mepc,t0
	lw	t0,8(sp)
	csrw	mpie,t0
	lw	t0,4(sp)
	lw	tp,16(sp)
	lw	gp,20(sp)
	lw	ra,24(sp)
	lw	sp,28(sp)
	mret
sysc_err:
	li	a1,-1
	j	sysc_sig

#
#	We need to save t1-t6 and a1-a7 somewhere as the signal handler is
#	entitled to mash them, and then restore them in the signal stub,
#	also need to figure out how to restore all regs and still branch
#	back into the old code wiht no reg changes. RISC-V exception
#	handling is seriously ugly stuff.
#	
return_via_signal:
	#	Modify the return path for signals
	#	On entry kstack holds the tp/gp/ra of the caller and the old
	#	sp. We need to magic up a stack frame to unwind
	lw	t0,28(sp)		# User SP
	addi	t0,t0,-32		# Make a scratch frame on the user stack
	# Top of the frame is the recovery tp/gp/ra
	lw	t1,16(sp)		# TP
	sw	t1,0(t0)
	lw	t1,20(sp)		# GP
	sw	t1,4(t0)
	lw	t1,24(sp)		# RA
	sw	t1,8(t0)
	csrr	t1,mepc
	sw	t1,12(t0)		# old PC
	sw	a0,16(t0)		# errno
	sw	a1,20(t0)		# retval
	lw	t1,U_DATA__U_CURSIG(tp)
	add	t1,t1,t1
	add	t1,t0,U_DATA__U_CODEBASE
	lw	t2,0(t1)
	sw	zero,0(t1)
	beq	t2,zero,sysc_ret # raced
	# Our mret will now go to signal handler, which will see an ra of
	# the unwind. which will pull stuff back off the stack
	csrw	mepc,t2
	# We are going to do this, adjust the return stack ptr info
	sw	t0,28(sp)	
	# Return to the stub helper on signal completion
	lw	t0,U_DATA__U_CODEBASE(tp)
	sw	t0,24(sp)
	j	sysc_ret

signal_stub:
	#	We arrive here after the signal was handled. This is run as
	# 	userspace code so on protected modes needs to be in stubs or
	#	somewhere suitable if we add protection support. TODO
	lw	tp,0(sp)
	lw	gp,4(sp)
	lw	ra,8(sp)
	lw	t0,12(sp)		# Address to jump to
	lw	a0,16(sp)
	lw	a1,20(sp)
	add	sp,sp,32		# Throw the signal frame
	jr	t0
