#include "../../cpu-riscv32/kernel-riscv32.def"
#
#	Called with interrupts off and in supervisor mode
#

	.globl _end
	.globl _bss_start

	.text

#if 0
head:
		.ascii "BOOTRV32"

		addr	head(pc),r3

		# Relocate ourselves down correctly
		movd	0x10,r0
		addr	relocate(pc),r1
		movd	0x1f000,r2
		movsd
		jump	0x1f000
#
#	This little block is run high up
#
relocate:
	movd	0x4000,r0
	movd	r3,r1
	movd	0x8000,r2
	movsd
	jump	start

#
#	And this is where we enter the code in proper location
#
#endif

start:
	.option	nopic
	.option push
	.option	norelax
	la	gp, __global_pointer$
	.option	pop

	la	t0,__bss_start
	la	t1, __end
wipebss:
	sw	zero,0(t0)
	addi	t0,t0,4
	blt	t0,t1,wipebss

	la	tp,udata_block
	addi	sp,tp,UBLOCK_SIZE-4

	jal	init_early
	jal	init_hardware
	jal	fuzix_main
	j	plt_monitor

	.data

	.long 0xAABBCCDD
