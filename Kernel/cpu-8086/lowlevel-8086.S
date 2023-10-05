	.arch i8086,jumps
	.arch .8087
	.code16
	.att_syntax prefix

	.text

	.globl unix_syscall_entry
	.globl udata_ptr
	.globl doexec
	.globl __hard_di
	.globl __hard_irqrestore
	.globl __hard_ei
	.globl set_irq
	.globl cpu_detect
	.globl set_cpu_type
	.globl outstring

	.globl copy_to_far
	.globl copy_from_far

	/* GCC glue */
	.globl __ashrsi3
	.globl	__ashlsi3
	.globl abort

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
 *
 *	Useful to know: an interrupt pushes the words in this order
 *	flags, segment, ip
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

	movw	udata_ptr, %bx
	movw	%ax, U_DATA__U_CALLNO(%bx)

	movw	10(%bp),%ax
	movw	%ax,U_DATA__U_ARGN(%bx)
	movw	12(%bp),%ax
	movw	%ax,U_DATA__U_ARGN1(%bx)
	movw	14(%bp),%ax
	movw	%ax,U_DATA__U_ARGN2(%bx)
	movw	16(%bp),%ax
	movw	%ax,U_DATA__U_ARGN3(%bx)

	/*
	 * Time to stack switch (we need SP for brk() even though the
	 * entry/exit code has no need of it and can use bp
	 */
	movw	%sp,%ax
	movw	%ax,U_DATA__U_SYSCALL_SP(%bx)

	/*
	 * Stack switch. Load sp the instruction after ss. Doesn't matter
	 * currently as IRQs are off, but may if this changes
	 */
	movw	%ds,%ax
	movw	%ax,%ss
	/*
	 *	Kstack is at udata + 512
	 */
	movw	%bx,%ax
	addw	$UBLOCK_SIZE,%ax
	movw	%ax,%sp

	movb	$1,U_DATA__U_INSYS(%bx)

	cld

	sti	
	call	unix_syscall
	cli

	/* At this point the user cs,ds,ss may have moved */

	movw	udata_ptr, %bx
	movb	$0,U_DATA__U_INSYS(%bx)

	/*
	 *	We have a pending signal ?
	 */

	cmpb	$0,U_DATA__U_CURSIG(%bx)
	je	no_signal

	xorb	%ah,%ah
	movb	+U_DATA__U_CURSIG(%bx),%al
	movb	$0, U_DATA__U_CURSIG(%bx)
	movw	U_DATA__U_SIGVEC(%bx),%bx
	addw	%ax,%bx
	addw	%ax,%bx
	movw	%bx,%cx

	/*
	 *	Build a suitable return frame
	 */

	pushw	%ax	/* Save signal number */

	movw	U_DATA__U_PAGE+2(%bx),%ax
	movw	%ax,%es
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
	   
	movw	U_DATA__U_RETVAL(%bx),%ax
	movw	%ax,-2(%bp)
	movb	U_DATA__U_ERROR(%bx),%al
	xorb	%ah,%ah
	movw	%ax,-4(%bp)
	movw	$23,-6(%bp)
	pop	%ax		/* Signal number */
	movw	%ax,-8(%bp)
	movw	U_DATA__U_PAGE(%bx),%ax
	movw	%ax,-10(%bp)
	movw	%cx,-12(%bp)
	/* FIXME: need to fix MMU logic */
	subw	$-14,%bp
	movw	%es,%ax
	movw	%ax,%ds
	movw	%ax,%ss
	movw	%bp,%sp
	/* To signal handler */
	sti
	retf
no_signal:
	/*
	 * We effective return a 32bit ulong to gcc half of which is return
	 * and error
	 */
	xorb	%dh,%dh
	movb	U_DATA__U_ERROR(%bx),%dl
	movw	U_DATA__U_RETVAL(%bx),%ax

	/* FIXME: need MMU code here to get new correct cs/ds/es */

	movw	U_DATA__U_PAGE(%bx), %cx
	movw	U_DATA__U_PAGE+2(%bx),%bx

	/* We can't reference kernel objects from here */
	movw	%bx, %ss
	movw	%bx, %ds
	movw	%bp, %sp
	movw	%cx,4(%bp)	/* Patch the cs for the iret - we might
				   have been relocated */
	popw	%bp		/* Recover frame pointer */
	iret

/*
 *	AX is the address to jump to
 *
 *	FIXME: this completely disagrees with the C code over page/page2
 *	for the moment. This is while we get it building and then worry
 *	about the mmu struct glue
 */
doexec:
	movw	%sp,%bp
	movw	2(%bp),%ax
	cli
	movw	udata_ptr,%bx
	movb	$0,U_DATA__U_INSYS(%bx)

	/* SS:SP to the user stack segment */
	movw	U_DATA__U_PAGE2(%bx),%dx
	movw	%dx, %ss
	movw	U_DATA__U_ISP(%bx),%dx
	movw	%dx,%sp
	/*
	 * Stack the new CS:IP
	 */
	movw	U_DATA__U_PAGE(%bx),%dx
	pushw	%dx	/* CS: */
	pushw	%ax	/* IP */
	/*
	 * Load the data segment into ES
	 */
	movw	U_DATA__U_PAGE2(%bx),%dx
	movw	%dx, %es
	/*
	 * Just ES and DS left to go
	 */
	movb	$0,U_DATA__U_INSYS(%bx)

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

trap_signal:
	movw	udata_ptr,%bx
	movw	U_DATA__U_PTAB(%bx), %ax
	/* TODO */
	jmp ssig

outstring:
outstringhex:
outax:
outcharhex:	
	ret

__hard_di:
	pushf
	popw	%ax
	cli
	ret

__hard_irqrestore:
	popw	%bx
	popw	%ax
	pushw	%ax
	pushw	%bx
	pushw	%ax
	popf
	ret

__hard_ei:
	sti
	ret

set_irqvec:
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

/*
 *	Useful differences
 *	8088/86	- cannot modify top 4 bits of flags, 80826 can
 *	8088/8086/80186 - push sp is buggy and pushes the wrong value
 *	8088/86 - word write to xx:FFFF wraps, 80186 doesn't wrap, 80286
 *		  traps
 *
 *	This lot needs to end up in discard
 *
 *	Returns
 *	0	8088/6
 *	1	80C88/C86
 *	2	NEC V20/30
 *	3	186
 *	4	286 or higher
 *
 *	We don't bother trying to tell 186/188 and 86/89 bus width as we
 *	don't need to know.
 *
 *	If we need to we can detect an early 8086 by setting a trace trap
 *	and seeing if we take it on pop of a segment register. If we do it's
 *	an early one. If it doesn't then its a later one (and if it also
 *	doesn't take one on push of a segment register its a Harris CMOS one
 */

test_cputype:
	/* See if shifts wrap - if so it's an 8086 or 8088 */
	movw	$0x121,%cx
	shl	%cl,%ch
	jne	is808x

	/* So it's a 186 or better. See if the push sp quirk is fixed */
	pushw	%sp
	popw	%bx
	cmpw	%bx,%sp
	/* 286 or better fix the push sp funny */
	jne	is8028x
	/* 186 ? */
	movb	$3,%al
	ret
is808x:
	/* See if we have an NEC V20/V30 */
	xorb	%al,%al		/* Z */
	movb	$40,%al
	mul	%al
	jne	notnec
	movb	$2,%al
	ret
notnec:
	/* See if we have the prefix rep fail bug. On a non CMOS 8086
	   the CPU restarts the instruction only allowing for the last
	   prefix - so the rep is forgotten only the es is used. interrupts
	   must be enabled and the timer running for this check */
	/* Needs to be enough loops that we guarantee an IRQ hits but not
	   too many more */
	movw	$0xffff,%cx
	rep	lodsb %es:(%si)	/* Spin for IRQ */
	jcxz	cmos86
	xorw	%ax,%ax
	ret
cmos86:
	movb	1,%al
	ret
is8028x:
	/* 286 or higher, We can trivially check for 386 but we just
	   don't care about it */
	movb	$4,%al
	ret

/*
 *	Call this only for older processors. It won't give a valid
 *	answer for a 286. It can only be run once and self modifies.
 */
bus_width_test:
	pushw %di
	xorw %dx,%dx
	pushf
	pushw %ds
	cli
	movw %cs,%ax
	movw %ax,%ds
	lea patch+2,%di
	movb $0x90,%al
	movw $3,%cx
	std
	rep stosb
	nop
	nop
	nop
	nop
patch:	inc %dx
	nop
	nop
	popw %ds
	popf
	popw %di
	testw %dx,%dx
	je eightbit
	movb $16,bus_width
	ret
eightbit:
	movb $8,bus_width
	ret

test_fputype:
	fninit
	movw	$0x55AA, scratch
	fnstsw	scratch
	cmpb	$0,scratch
	jne	no_fpu
	fnstcw	scratch
	movw	scratch,%ax
	andw	$0x103F,%ax
	cmpw	$0x3F,%ax
	jne	no_fpu
	andw	$0xff7f,%ax	/* clear interrupt bit - FIXME do we need ? */
	fldcw	scratch
	fdisi			/* 8087 only */
	fstcw	scratch
	testb	$80,scratch	/* Did the int bit change */
	jnz	fpu_8087
	/* 287 or better - do we care about 387 probably not */
	movb	$2, fpu_type
	ret
fpu_8087:
	movb	$1, fpu_type
	ret
no_fpu:
	movb	$0, fpu_type

/*
 *	Must be called with interrupts on and a timer running. When this
 *	completes we know the processor type, the fpu type (if any) and
 *	the bus width 8 v 16bit
 *
 *	We don't detect anything beyond 286 because we don't care about
 *	any features beyond that. Detecting processor clock rate is rather
 *	tricky (the PC/AT for example runs at 1 wait state) and we don't
 *	really have a use for that either - so we don't.
 */

cpu_detect:
	call	test_fputype
	call	test_cputype
	movb	%al,cpu_type
	cmpb	$4,%al
	je	always_16bit
	jmp	bus_width_test
always_16bit:
	movb	$16,bus_width
	ret

/* TODO : wrap cpu_detect */
set_cpu_type:
	ret
/*
 *	Handy routines for block copying to/from far memory. These
 *	are not optimized. We should be smarter about using movsw
 *	and alignment on 8086+
 *
 *	Warning: don't use prefixes with rep as we have to run on 8086
 *
 *	copy_to_far(int16_t seg, int16_t to, void *from, int16_t len)
 *	copy_from_far(int16_t seg, void *to, int16_t from, int16_t len)
 */

copy_to_far:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	pushw	%si
	pushw	%di
	pushw	%ds
	movw	4(%bp),%ax
	movw	%ax,%es
copier:
	movw	6(%bp),%di
	movw	8(%bp),%si
	movw	10(%bp),%ax
	movw	%ax,%cx
	shr	%cx
	jz	noblock
	rep	movsw
noblock:
	testb	$1,%al
	jne	nofinal
	movsb
nofinal:
	popw	%ds
	popw	%di
	popw	%si
	popw	%es
	popw	%bp
	ret

copy_from_far:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	pushw	%si
	pushw	%di
	pushw	%ds
	movw	4(%bp),%ax
	movw	%ax,%ds
	jmp	copier

/* FIXME: extract from C library or write nice ones */
__ashlsi3:

__ashrsi3:
	ret

/* FIXME */
abort:
	jmp plt_monitor

	.data

scratch:
	.word	0
fpu_type:
	.byte	0
bus_width:
	.byte	0
cpu_type:
	.byte	0

	/* TODO */
	.globl	sys_cpu
	.globl	sys_cpu_feat
	.globl	sys_stubs

sys_cpu:
	.byte	0
sys_cpu_feat:
	.byte	0
sys_stubs:
	.ds	32