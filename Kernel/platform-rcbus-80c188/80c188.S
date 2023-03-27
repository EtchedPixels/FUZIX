	.text
	.globl program_vectors
	.globl plt_monitor
	.globl plt_reboot
	.globl outchar
	.globl init_hardware
	.globl init_early

init_early:
program_vectors:
	ret
	
plt_monitor:
plt_reboot:
	jmp plt_reboot


	.text

/*
 *	KL11 and equivalents BR 4  vec 064 (tx) 060 (rx)
 */ 

outchar:
	ret

init_hardware:
	/* FIXME: set up traps */
	/* FIXME: make up some numbers for the moment */
	/* FIXME: interrupts etc */
	movw	$512,%ax
	movw	%ax,ramsize
	subw	$64,%ax
	movw	%ax,procmem
	ret

	.data

	.globl	udata_ptr

udata_ptr:
	.word	0