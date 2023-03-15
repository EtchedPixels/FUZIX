#include "../kernel-ns32k.def"
/*
 *	Lots left to fill in
 */
	.globl	_plt_reboot
	.globl	init_early
	.globl	init_hardware
	.globl	_program_vectors
	.globl	outchar
	.globl	_plt_monitor
	.globl	_udata_block
	.globl	_get_usp
	.globl	_set_usp

#
#	Put early as needs to be in the low 64K
#

	.align	2
module:
sbase:	.long	0
	.long	0
	.long	0
	.long	0
inttab:
	# Non vectored external IRQ (all in our case)
	.word	0
	.word	interrupt_handler
	# Non maskable interrupt (not wired)
	.word	0
	.word	nmi_handler
	.word	0
	.word	abort_handler
	.word	0
	.word	slave_handler
	.word	0
	.word	illegal_handler
	.word	0
	.word	syscall_handler
	.word	0
	.word	divzero_handler
	.word	0
	.word	flag_handler
	.word	0
	.word	breakpoint_handler
	.word	0
	.word	trace_handler
	.word	0
	.word	undefined_handler
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	# Vectored interrupts follow (non used)


_plt_reboot:
_plt_monitor:
	sprw	psr,r0		# Interrupts back off
	andw	0xF7FF,r0
	lprw	psr,r0
	br	_plt_monitor

init_early:
	addr	_udata_block(pc),_udata_shadow(pc)
	addr	inttab,r0
	lprd	intbase,r0
	addr	module,r0
	lprw	mod,r0
	movd	sbase,r0
	lprd	sb,r0
	ret	0

#
#	FIXME: could be in discard if we wanted
#
init_hardware:
# set system RAM size(hardcode hacks for now) TODO
	movw	1024,_ramsize(pc)
	movw	1024-64,_procmem(pc)		# guesses for now
	bsr	_program_vectors
# TODO timer enable etc
	ret	0

#
#	Nothing to do in NS32K - all set up once at boot
#
_program_vectors:
	ret	0

#
#	We do no banking so we need to do nothing here.
#
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	ret	0


# outchar: Wait for UART TX idle, then print the char in r0

outchar:
	movw	r1,tos
outcharw:
	movb	0xF0018A(pc),r1
	andb	0x20,r1
	beq	outcharw
	movb	r0,0xF00180(pc)
	movw	tos,r1
	ret	0


#
#	Later versions of the CPU will need these
#
_flush_icache:
#	cinv	A,I,r0
#	ret	0
_flush_dcache:
#	cinv	A,D,r0
#	ret	0
_enable_icache:
_enable_dcache:
	ret	0

_get_usp:
	sprd	usp,r0
	ret	0
_set_usp:
	enter	[],0
	movd	4(fp),r0
	lprd	usp,r0
	ret	0
