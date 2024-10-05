	.arch i8086,jumps
	.code16
	.att_syntax prefix
	.data

	.global udata
	.global kstack_top
	.global istack_top

#include	"../kernel-8086.def"

udata:
	.bss 512
kstack_top:
	.bss 256
istack_top:

	.text

	.global init_early
	.global init_hardware
	.global program_vectors
	.global plt_monitor
	.global plt_reboot
	.global kernel_ds

	.global bioshd_param
	.global bioshd_read
	.global bioshd_write
	.global bioshd_reset
	.global biosdata_read
	.global equipment_word

	.global	do_copy_page
	.global do_zero_page

init_early:
	/* Dig useful information out of the BIOS */
	int $11
	mov %ax,equipment_word
	int $12
	mov %ax,ramsize
	ret
init_hardware:
	ret
program_vectors:
	ret
plt_monitor:
plt_reboot:
	jmp plt_monitor

/*
 *	Interrupt and trap handlers. The 8086 doesn't have real protection
 *	but it does at least actually throw real traps for the things it
 *	does which makes it easier to handle
 */

int1:
	pushw	%ax
	movb	$1,%al
	jmp	do_irq
int2:
	pushw	%ax
	movb	$2,%al
	jmp	do_irq
int3:
	pushw	%ax
	movb	$3,%al
	jmp	do_irq
int4:
	pushw	%ax
	movb	$4,%al
	jmp	do_irq
int5:
	pushw	%ax
	movb	$5,%al
	jmp	do_irq
int6:
	pushw	%ax
	movb	$6,%al
	jmp	do_irq
int7:
	pushw	%ax
	movb	$7,%al
	jmp	do_irq
int8:
	pushw	%ax
	movb	$8,%al
	jmp	do_irq
int9:
	pushw	%ax
	movb	$9,%al
	jmp	do_irq
int10:
	pushw	%ax
	movb	$10,%al
	jmp	do_irq
int11:
	pushw	%ax
	movb	$11,%al
	jmp	do_irq
int12:
	pushw	%ax
	movb	$12,%al
	jmp	do_irq
int13:
	pushw	%ax
	movb	$13,%al
	jmp	do_irq
int14:
	pushw	%ax
	movb	$14,%al
	jmp	do_irq
int15:
	pushw	%ax
	movb	$15,%al
	jmp	do_irq
int0:
	pushw	%ax
	xorb	%al,%al

	/* FIXME: we need a way to cleanly hook the schedule/signal parts
	   of this into an int handler for the BIOS timer callback */
do_irq:
	/* Save base registers on current stack */
	pushw	%ds
	pushw	%es
	pushw	%bx
	pushw	%cx
	pushw	%dx
	pushw	%bp


	/* Reload segment registers */
	movw	%cs:kernel_ds,%bx
	movw	%bx,%ds
	movw	%bx,%es

	/* At this point our ds/es/cs are all kernel but our SS could be
	   one of several stacks */
	movw	%ss,%dx
	movw	%sp,%bp

	/* Switch to the IRQ stack and save the old stack */
	movw	%bx,%ss
	movw	$istack_top,%sp
	pushw	%dx
	pushw	%bp

	movb	$1,inint
	movb	$1,udata + U_DATA__U_ININTERRUPT
	pushw	%ax
	call	do_plt_interrupt
	popw	%ax

	/* This little chunk is machine specific so we'll need to work out
	   how to move most of this to lowlevel-8086 and keep the 8259A bits
	   behind */
	cmpb	$16,%al
	jb	internal	/* CPU generated */
	cmpb	$8,%al
	movb	$0x20,%al
	jb	xt_int
	outb	%al,$0xA0
	/* The 8259 is slower than the 8086 so we need to wait */
	jmp	delay
delay:	jmp	xt_int
xt_int:
	outb	%al,$0x20
internal:
	movb	$0,inint
	movb	$0,udata + U_DATA__U_ININTERRUPT

	cmpb	$1,udata + U_DATA__U_INSYS
	je	ret_user

	/* Pull the data off the stack and life continues */
	popw	%bp
	popw	%dx

	movw	%bp,%sp
	movw	%dx,%ss

	popw	%bp
	popw	%dx
	popw	%cx
	popw	%bx
	popw	%es
	popw	%ds
	iret

	/* The user path is more complicated. We might have been signalled
	   or need to task switch. At this point we are on the kernel stack */
ret_user:
	cmpb	$0,need_resched
	je	no_resched

	popw	%bp
	popw	%dx

	/* 
	 * Save our data for the user stack where the registers are saved. This
	 * matters big time because the istack will get reused before
	 * we re-run. Instead we borrow our kstack (which is private and -
	 * as we are not in a syscall free for use
   	 */
	movw	%cs:kernel_ds,%ss
	movw	$kstack_top,%sp

	pushw	%dx
	pushw	%bp

	movw	udata + U_DATA__U_PTAB,%bp
	movb	$P_READY,P_TAB__P_STATUS_OFFSET(%bp)

	call	switchout

	/* Another task will start execution, time will pass and eventually
	 * we'll pop back out of here */

no_resched:
	popw	%bp
	popw	%dx
	movw	%dx,%ss
	movw	%bp,%sp

	/* Back on our user stack */

	/* We are now on the user stack via ss
	/* Dispatch signals if required */
	movb	udata + U_DATA__U_CURSIG,%al
	cmpb	$0,%al
	je	no_signal
	/*
	 *	We have a pending signal
	 */
	movb	$0,udata + U_DATA__U_CURSIG;
	xorb	%ah,%ah
	movw	udata + U_DATA__U_SIGVEC,%bp
	addw	%ax,%bp
	addw	%ax,%bp
	movw	(%bp),%bx
	movw	$0,(%bp)

	/*
	 *	Build a deeper return frame
	 */
	movw	$20,%ax		/* __sigeturn in binary */
	pushw	%ax
	pushf			/* IRET frame */
	pushw	%bx
	/* FIXME: we need to wire the MMU code in here and get the *new*
	   correct CS */
	movw	4(%bp),%ax	/* Get CS from original frame */
	pushw	%ax		/* And push it */
	/* Clean registers */
	xorw	%ax,%ax
	xorw	%bx,%bx
	xorw	%cx,%cx
	xorw	%dx,%dx
	xorw	%bp,%bp
	iret

	/* _sigreturn does
	   recover registers including flags
	   skip over spare cs: ret */

	/* The usual path is uneventful */
no_signal:
	popw	%bp
	pop	%dx
	pop	%cx
	pop	%bx
	popw	%es
	popw	%ds
	popw	%ax
	iret


/* Returns the BIOS parameter data CX:DX as a long DX:AX to the C code or
   FFFF if we don't get the right answer

   Things to note this being PC BIOS land
	- bl return is not reliable for floppies (nor sometimes %cx)
	- es:di is not reliably set by the BIOS
	- you must do an int 13 ah=0x01 after this call on PS/2 model 30
	- some junk leaves interrupts disabled after this call
	- Leading Edge BIOSes may trash di/si/bp/ds/es !
	- Some Compaq BIOSes will make stuff up for drive numbers above
	  the last one we use

    long bioshd_param(int drive)
*/
bioshd_param:
	pushw	%bp
	movw	%sp,%bp
	movw	$0x0800,%ax		/* get drive parameters */
	movw	4(%bp),%dx
	pushf
	stc
	pushw	%di
	pushw	%si
	pushw	%bp
	pushw	%ds
	int	$13
	popw	%ds
	movw	%ds,%ax
	movw	%ax,%es
	popw	%bp
	popw	%si
	popw	%di
	jc	bioshd_fail
	popf
	pushw	%cx
	pushw	%dx
	movw	4(%bp),%dx		/* drive number */
	movb	$1,%ah
	int	$13
	popw	%dx
	popw	%ax
	popw	%bp
	ret
bioshd_fail:
	popf
	popw	%bp
	mov	$0xFF,%ax
	mov	%ax,%dx
	ret

bioshd_status:
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%dx
	movb	$1,%ah
	int	$13
	popw	%bp
	ret
	
bioshd_reset:
	xorb	%ah,%ah
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%dx
	int	$13
	popw	%bp
	ret

/*
 *	Attempt a block read
 *
 *	Guess what this is also horribly buggy on lots of systems too
 *	Buffer must be even on some AMI BIOSen (1990/1) so we need to
 *	handle direct I/O with care and also FIXME align buffer cache
 *
 *	Some bioses destroy dx, and don't always handle carry right so we
 *	must call with stc first
 *
 *	The original IBM PC-AT BIOS doesn't correctly handle interrupt
 *	disables. For now we just treat that as busted - use a newer BIOS,
 *	you've had 30 years to upgrade it!
 *
 *	bioshd_read(uint16_t cylsec, uint8_t drive, uint8_t head,
 *			uint16_t seg, uint16_t addr, uint16_t secs)
 */
bioshd_read:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	movw	4(%bp),%cx
	movw	6(%bp),%dx
	movw	8(%bp),%ax
	movw	%ax,%es
	movw	10(%bp),%bx
	movw	12(%bp),%ax
	movb	$2,%ah
	stc
	int	$13
	jc	read_fail
	movw	$0,%dx
read_end:
	popw	%es
	popw	%bp
	ret
read_fail:
	movw	$0xff,%dx
	jmp	read_end
	

bioshd_write:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	movw	4(%bp),%cx
	movw	6(%bp),%dx
	movw	8(%bp),%ax
	movw	%ax,%es
	movw	10(%bp),%bx
	movw	12(%bp),%ax
	movb	$3,%ah
	stc
	int	$13
	jc	read_fail
	movw	$0,%dx
	popw	%es
	popw	%bp
	ret

biosdata_read:
	pushw	%bp
	movw	%sp,%bp
	pushw	%di
	movw	$0x40,%ax
	movw	%ax,%es
	movw	4(%bp),%di
	movb	(%di),%al
	cbw
	movw	%ds,%bx
	movw	%bx,%es
	popw	%di
	popw	%bp
	ret

do_copy_page:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	pushw	%ds
	pushw	%si
	pushw	%di
	movw	4(%bp),%ax
	movw	%ax,%es
	movw	6(%bp),%ax
	movw	%ax,%ds
	xorw	%si,%si
	movw	%si,%di
	movw	$0x0800,%cx	/* Change me if not using 4K pages */
	rep	movsw
	popw	%di
	popw	%si
	popw	%ds
	popw	%es
	popw	%bp
	ret

do_zero_page:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	pushw	%di
	movw	4(%bp),%ax
	movw	%ax,%es
	movw	6(%bp),%ax
	movw	%ax,%ds
	xorw	%ax,%ax
	xorw	%ax,%si
	movw	%ax,%di
	movw	$0x0800,%cx	/* Change me if not using 4K pages */
	rep	stosw
	popw	%si
	popw	%es
	popw	%bp
	ret
	
kernel_ds:
	.word 0


