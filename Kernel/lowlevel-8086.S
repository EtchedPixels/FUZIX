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
	.global set_irq

	/* GCC glue */
	.global __ashrsi3
	.global	__ashlsi3
	.global abort

#include "kernel-8086.def"

/*
 *	Things to be careful of
 *
 *	1. We run the syscall on this tasks kernel stack
 *	2. We can enter into here, schedule and pop out of someone else's
 *	   stack so don't use globals without thought
 *	3. The cs we trap from is not always the cs we return to, nor
 *	   is ss/ds guaranteed to stay the same as the syscall might
 *	   (eg brk) cause us to be moved phsically.
 *
 *	Rules
 *
 *	1. We always make a syscall from user space
 *	2. We never make a syscall with interrupts off
 *	3. We treat the compiler scratch registers as fair game
 */

unix_syscall_entry:
	pushw	%bp			/* Create a stack frame */
	movw	%sp, %bp
	movw	%cs:kernel_ds,%bx	/* Load our DS */
	movw	%bx, %ds
	movw	%bx, %es

	/*
	 * We are now referencing the kernel except for the stack
	 * and that has our arguments on it with 6 bytes of trap
	 * return info before it and the segment registers we
	 * added plus a return (*). bp will reference ss: so we can now stash
	 * away our arguments in peace
	 *
	 * (*) The extra return means read (A,B,C) can just do mov $foo,%ax
	 * and int, and we'll use the parent stack frame to avoid copies
	 */

	movw	%ax, udata+U_DATA__U_CALLNO

	movw	10(%bp),%ax
	movw	%ax,udata+U_DATA__U_ARGN
	movw	12(%bp),%ax
	movw	%ax,udata+U_DATA__U_ARGN1
	movw	14(%bp),%ax
	movw	%ax,udata+U_DATA__U_ARGN2
	movw	16(%bp),%ax
	movw	%ax,udata+U_DATA__U_ARGN3

	/*
	 * Time to stack switch (we need SP for brk() even though the
	 * entry/exit code has no need of it and can use bp
	 */
	movw	%sp,%ax
	movw	%ax,udata+U_DATA__U_SYSCALL_SP

	/*
	 * Stack switch. Load sp the instruction after ss. Doesn't matter
	 * currently as IRQs are off, but may if this changes
	 */
	movw	%ds,%ax
	movw	%ax,%ss
	movw	kstack_top,%sp

	movb	$1,kernel_flag

	sti	
	call	unix_syscall
	cli

	/* At this point the user cs,ds,ss may have moved */

	movb	$0,kernel_flag

	/*
	 *	We have a pending signal ?
	 */

	cmpb	$0,udata+U_DATA__U_CURSIG
	je	no_signal

	xorb	%ah,%ah
	movb	udata+U_DATA__U_CURSIG,%al
	movb	$0, udata+U_DATA__U_CURSIG
	movw	udata+U_DATA__U_SIGVEC,%bx
	addw	%ax,%bx
	addw	%ax,%bx
	movw	%bx,%cx

	/*
	 *	Build a suitable return frame
	 */

	movw	udata+U_DATA__U_PAGE+2,%bx
	movw	%bx,%es
	/* ES:BP is now the user stack. We build a new frame below
	   the existing one that looks like this

	
	   retval
	   error code
           $23 (signal_return) handler
	   signalnumber
	   flags
	   signalvector
	   cs

	   The handler pops the retval/error into ax/dx then
	   dumps the stale iret frame segment and does a ret
	*/
	   
	movw	udata+U_DATA__U_RETVAL,%ax
	movw	%ax,-2(%bp)
	movb	udata+U_DATA__U_ERROR,%al
	xorb	%ah,%ah
	movw	%ax,-4(%bp)
	movw	$23,-6(%bp)
	pushf
	/* FIXME: enable interrupts in mask */
	popw	%ax
	movw	%ax,-10(%bp)
	movw	%cx,-12(%bp)
	/* FIXME: need to fix MMU logic */
	movw	udata+U_DATA__U_PAGE,%ax
	movw	%ax,-14(%bp)
	subw	$-14,%bp
	movw	%es,%ax
	movw	%ax,%ds
	movw	%ax,%ss
	movw	%bp,%sp
	/* To signal handler */
	iret
no_signal:
	/*
	 * We effective return a 32bit ulong to gcc half of which is return
	 * and error
	 */
	xorb %dh,%dh
	movb udata+U_DATA__U_ERROR,%dl
	movw udata+U_DATA__U_RETVAL,%ax

	/* FIXME: need MMU code here to get new correct cs/ds/es */

	movw udata+U_DATA__U_PAGE+2,%bx
	movw udata+U_DATA__U_PAGE, %cx

	/* We can't reference kernel objects from here */
	movw %bx, %ss
	movw %bx, %ds
	movw %bp, %sp
	movw %cx,4(%bp)		/* Patch the cs for the iret - we might
				   have been relocated */
	popw %bp		/* Recover frame pointer */
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

set_irq:
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%bx
	movw	6(%bp),%ax
	addw	%bx,%bx
	addw	%bx,%bx
	pushw	%es
	xorw	%cx,%cx
	movw	%cx,%es
	movw	%ax,%es:(%bx)
	movw	%bp,%sp
	ret

	


/* FIXME: extract from C library or write nice ones */
__ashlsi3:

__ashrsi3:
	ret

/* FIXME */
abort:
	jmp trap_monitor
