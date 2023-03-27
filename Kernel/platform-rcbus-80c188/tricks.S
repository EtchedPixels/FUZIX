#include "../kernel-8086.def"

	.arch i8086,jumps
	.code16
	.att_syntax prefix
	.text

	.text

	.globl switchin
	.globl plt_switchout
	.globl dofork

plt_switchout:
	/* All the segments are dealt with entering and exitign user */
	/* space so are not an issue for us here. */
	pushw	%bp
	pushw	%si
	pushw	%di
	xorw	%ax,%ax
	pushw	%ax
	/* It would be nice to be able to use %es: for udata in C
	   but the compiler has no such provision */
	movw	udata_ptr,%bx
	movw	%sp,U_DATA__U_SP(%bx)
	call	getproc
	/* ax is now the process to run */
	push	%ax
	call	switchin
	jmp	plt_monitor

	/* All the real magic happens on kernel entry and exit */
switchin:
	/* No need to save stuff here - we will return in another context */
	cli
	/* FIXME: need to clean up stack of argument ?? */
	movw	%sp,%bp
	movw	2(%bp),%si
	movw	udata_ptr,%bx
	
	cmpw	 $0,P_TAB__P_PAGE_OFFSET(%si)
	jne	not_swapped
	pushw	%bx
	movw	U_DATA__U_PTAB(%bx),%di
	cmpw	$0,P_TAB__P_PAGE_OFFSET(%di)

	/* FIXME: irqs on for swap needed yet */
	je	its_dead_jim
	pushw	%si
	call	swapout
	popw	%ax
its_dead_jim:
	/* Swap in our process */
	movw	%sp,%ax
	movw	$swapstack+256,%sp
	pushw	%ax	/* Old sp */
	pushw	%di
	call	swapper
	popw	%di

	/* TODO _ fix up page ptrs */
	movw	$1,P_TAB__P_PAGE_OFFSET(%di)
	popw	%sp
	popw	%bx
not_swapped:
	cli
	cmpw	U_DATA__U_PTAB(%bx),%di
	jne	switchfail
	movb	$P_RUNNING,P_TAB__P_STATUS_OFFSET(%di)
	xorw	%ax,%ax
	movw	%ax, runticks
	movw	U_DATA__U_SP(%bx),%sp
	popw	%ax
	popw	%di
	popw	%si
	popw	%bp
	ret

switchfail:
	mov	$badswitchmsg,%ax
	call	outstring		
	jmp	plt_monitor

dofork:
	push	%bp
	movw	%sp,%bp
	movw	4(%bp),%di	/* Child process info */
	pushw	%si
	pushw	%di
	movw	P_TAB__P_PID_OFFSET(%di),%ax
	pushw	%ax
	/* Stack frame now matches the switch frame */
	movw	udata_ptr,%bx
	movw	%sp, U_DATA__U_SP(%bx)
	movw	U_DATA__U_PTAB(%bx),%ax

	pushw	%bx
	pushw	%ax

	/* Copy the image, return in the child image */
	call	fork_copy

	call	makeproc
	addw	$4,%sp

	xorw	%ax,%ax
	movw	%ax,runticks
	ret

/* TODO Copy the udata and the user mappings */
fork_copy:
	ret

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13,10,0
swapstack:
	.ds	256
