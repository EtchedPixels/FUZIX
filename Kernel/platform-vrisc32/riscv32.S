#include "../kernel-riscv32.def"
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

# outchar: Wait for UART TX idle, then print the char in r0

outchar:
	jr	ra
#if 0
	movw	r1,tos
outcharw:
	movb	0xF0018A(pc),r1
	andb	0x20,r1
	beq	outcharw
	movb	r0,0xF00180(pc)
	movw	tos,r1
	ret	0
#endif
