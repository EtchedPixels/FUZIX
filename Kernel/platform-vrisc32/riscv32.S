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

plt_reboot:
plt_monitor:
	jal	di
	j	plt_reboot

init_early:
	la	t0,udata_block
	sw	t0,%lo(udata_shadow)(gp)
	jr	ra

#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now) TODO
	li	t0,1024
	sw	t0,%lo(ramsize)(gp)
	addi	t0,t0,-64
	sw	t0,%lo(procmem)(gp)
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
