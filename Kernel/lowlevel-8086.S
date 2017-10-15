	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

	.global unix_syscall_entry
	.global udata
	.global doexec
	.global __hard_di
	.global __hard_irqrestore
	.global __hard_ei

	/* GCC glue */
	.global __ashrsi3
	.global	__ashlsi3
	.global abort

#include "kernel-8086.def"

unix_syscall_entry:
	pushw %ds
	pushw %es

	pushw %bp
	movw %sp, %bp
	movw %cs:kernel_ds,%bx		/* Load our DS */
	movw %bx, %ds
	movw %bx, %es

	/*
	 * We are now referencing the kernel except for the stack
	 * and that has our arguments on it with 6 bytes of trap
	 * return info before it and two segment registers we
	 * added. bp will reference ss: so we can now stash away our
	 * arguments in peace
	 */

	movw %ax, udata+U_DATA__U_CALLNO

	movw 10(%bp),%ax
	movw %ax,udata+U_DATA__U_ARGN
	movw 12(%bp),%ax
	movw %ax,udata+U_DATA__U_ARGN1
	movw 14(%bp),%ax
	movw %ax,udata+U_DATA__U_ARGN2
	movw 16(%bp),%ax
	movw %ax,udata+U_DATA__U_ARGN3

	/*
	 * Time to stack switch (we need SP for brk() even though the
	 * entry/exit code has no need of it and can use bp
	 */
	movw %sp,%ax
	movw %ax,udata+U_DATA__U_SYSCALL_SP
/*	; save SS anyway - will be useful if we ever do multi-segment
	; binaries
	mov %ss,%ax
	mov %ax,udata+U_DATA__U_SYSCALL_SS(%bx) */

	/*
	 * Stack switch. Load sp the instruction after ss. Doesn't matter
	 * currently as IRQs are off, but may if this changes
	 */
	movw %ds,%ax
	movw %ax,%ss
	movw kstack_top,%sp

	movb $1,kernel_flag

	sti	
	call unix_syscall
	cli

	movb $0,kernel_flag

	/*
	 *	SIGNALS TODO
	 */

	/*
	 * We effective return a 32bit ulong to gcc half of which is return
	 * and error
	 */
	mov udata+U_DATA__U_ERROR,%dx
	mov udata+U_DATA__U_RETVAL,%ax

	/*
	 * Stack back
	 */
	mov udata+U_DATA__U_PAGE+3,%bx
	mov %bx, %ss
	mov %bp, %sp
	pop %es
	pop %ds
	iret

/*
 *	AX is the address to jump to
 *
 *	FIXME: this completely disagrees with the C code over page/page2
 *	for the moment. This is while we get it building and then worry
 *	about the mmu struct glue
 */
doexec:
	movw %sp,%bp
	movw 4(%bp),%ax
	cli
	movb $0,kernel_flag
	/*
	 * Stack the new CS:IP
	 */
	movw udata+U_DATA__U_PAGE2,%dx
	pushw %dx
	pushw %ax
	/*
	 * Load the initial stack
	 */
	movw udata+U_DATA__U_ISP,%dx
	movw %dx,%sp
	/* SS = DS */
	movw udata+U_DATA__U_PAGE,%dx
	movw %dx, %ss
	/*
	 * Load the data segment into ES
	 */
	movw udata+U_DATA__U_PAGE2,%dx
	movw %dx, %es
	/*
	 * Just ES and DS left to go
	 */
	movb $0,udata+U_DATA__U_INSYS
	/*
	 * And go
	 */
	movw %dx, %ds
	xorw %ax, %ax
	xorw %bx, %bx
	xorw %cx, %cx
	xorw %dx, %dx
	xorw %si, %si
	xorw %di, %di
	movw %ax, %bp
	sti			/* will occur after the retf completes */
	retf

interrupt_handler:
	/* TODO */
	iret

trap_signal:
	mov udata+U_DATA__U_PTAB, %ax
	jmp ssig

outstring:
outstringhex:
outax:
outcharhex:	
	ret

__hard_di:
	pushf
	popw %ax
	cli
	ret

__hard_irqrestore:
	popw %bx
	popw %ax
	pushw %ax
	pushw %bx
	pushw %ax
	popf
	ret

__hard_ei:
	sti
	ret

/* FIXME: extract from C library or write nice ones */
__ashlsi3:

__ashrsi3:
	ret

/* FIXME */
abort:
	jmp trap_monitor
