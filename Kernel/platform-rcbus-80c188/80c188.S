	.text
	.globl program_vectors
	.globl plt_monitor
	.globl plt_reboot
	.globl outchar
	.globl init_hardware
	.globl init_early

	.globl devide_read_data
	.globl devide_write_data

#include "../kernel-8086.def"

init_early:
program_vectors:
	ret
	
plt_monitor:
plt_reboot:
	jmp plt_reboot


	.text

/*
 *	The existing Fuzix setup deals with only 3 I/O targets at most
 *	(all of which are the same on a flat system). We add 3 as user code
 */
devide_read_data:
	movw	(blk_op + 2), %ax
	movw	udata_ptr,%bx
	movw	%ds,%cx
	cmpw	$0,%ax
	je	read_k
	movw	U_DATA__U_PAGE+2(%bx),%cx
	cmpw	$1,%ax
	je	read_u_d
	cmpw	$2,%ax
	jne	read_u_i
	movw	(blk_op + 4), %cx
	jmp	read_k
read_u_i:
	movw	U_DATA__U_PAGE(%bx),%cx
read_u_d:
read_k:
	push	%di
	movw	(blk_op),%di
	movw	%cx,%es
	movw	$0x10,%bx
	movw	$0x200,%cx
	rep	insb
	movw	%ds,%ax
	movw	%ax,%es
	pop	%di
	ret

devide_write_data:
	movw	(blk_op + 2), %ax
	movw	udata_ptr,%bx
	movw	%ds,%cx
	cmpw	$0,%ax
	je	write_k
	movw	U_DATA__U_PAGE+2(%bx),%cx
	cmpw	$1,%ax
	je	write_u_d
	cmpw	$2,%ax
	jne	write_u_i
	movw	(blk_op + 4), %cx
	jmp	write_k
write_u_i:
	movw	U_DATA__U_PAGE(%bx),%cx
write_u_d:
write_k:
	push	%di
	movw	(blk_op),%di
	movw	%cx,%es
	movw	$0x10,%bx
	movw	$0x200,%cx
	rep	outsb
	movw	%ds,%ax
	movw	%ax,%es
	pop	%di
	ret

outchar:
	ret

init_hardware:
	/* FIXME: set up traps */
	/* FIXME: make up some[A numbers for the moment */
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