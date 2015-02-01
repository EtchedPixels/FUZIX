/*
 *	We do most of this in C on the 68000
 */
#include "kernel-68000.def"

.mri 0
		.globl di,ei,irqrestore
		.globl doexec
		.globl outstring,outstringhex,outcharhex,outa0hex
.mri 1
get_usp:	move.l	usp,a0
		rts

set_usp:	move.l 4(sp),a0
		move.l a0,usp
		rts

/*
 *	We don't currently have any fine grained IRQ control. We need to fix
 *	that!
 */
di:
		move.w sr,d0
		and.w #$0700,d0
		or.w #$0700,sr
		rts

ei:
		and.w #$F8FF,sr
		rts

irqrestore:
		move.w sr,d0
		and.w #$F8FF,d0
		or.w 4(sp),d0
		move.w d0,sr
		rts

doexec:
		;
		;	FIXME: we need to reset sp (we effectively longjmp
		; 	in supervisor space), then push the values. If we
		;	don't then the supervisor stack will go awry
		;
		move.l (sp)+,a0			; throw the return address
		clr.w -(sp)			; set up the status register
		move.l U_DATA__U_ISP,a0
		move.l a0,usp			; set the user stack
		moveq #0,d0			; wipe the registers
		move.l d0,d1
		move.l d0,d2
		move.l d0,d3
		move.l d0,d4
		move.l d0,d5
		move.l d0,d6
		move.l d0,d7
		move.l d0,a0
		move.l d0,a1
		move.l d0,a2
		move.l d0,a3
		move.l d0,a4
		move.l d0,a5
		move.l d0,a6
		rte		; hit user space (and will enable interrupts)


bus_error:	move.w #SIGBUS,-(sp)
		bra sig_or_die
addr_error:	move.w #SIGSEGV,-(sp)
		bra sig_or_die
illegal:	move.w #SIGILL,-(sp)
		bra sig_or_die
divzero:	move.w #SIGFPE,-(sp)
		bra sig_or_die
chk:		move.w #SIGFPE,-(sp)
		bra sig_or_die
trapv:		move.w #SIGFPE,-(sp)
		bra sig_or_die
priv:		move.w #SIGFPE,-(sp)
		bra sig_or_die
trace:		move.w #SIGTRAP,-(sp)
		bra sig_or_die
unimpa:		move.w #SIGFPE,-(sp)
		bra sig_or_die
unimpf:		move.w #SIGFPE,-(sp)
		bra sig_or_die
misctrap:	move.w #SIGILL,-(sp)
		bra sig_or_die
trap15:		move.w #SIGTRAP,-(sp)			; by convention
		bra sig_or_die
trap14:		movem.l a2-a6/d2-d7,-(sp)
		move.b #1,d0
		move.b d0,kernel_flag
		; FIXME: EI per platform really
		and.w #$F8FF,sr
		bsr unix_syscall
		; FIXME: signal handling in the C code or not ?
	        or.w #$0700,sr
		move.w U_DATA__U_ERROR,d1
		bne sysc_err
		move.w U_DATA__U_RETVAL,d0
		movem.l (sp)+,a2-a6/d2-d7
		rte
sysc_err:	move.w #-1,d0
		movem.l (sp)+,a2-a6/d2-d7
		rte

spurious:	movem.l a0-a6/d0-d7,-(sp)
		move.l #strspurious,a0
		bsr outstring
		movem.l (sp)+,a0-a6/d0-d7
		rte

uninit:		movem.l a0-a6/d0-d7,-(sp)
		move.l #struninitialized,a0
		bsr outstring
		movem.l (sp)+,a0-a6/d0-d7
		rte

strspurious:	asciz "spurious interrupt\n"
struninitialized:
		asciz "uninitialized interrupt\n"

		.align 2
;
;	We do interrupts a bit differently - we use a single supervisor
; stack per process, not an interrupt stack. As a result our signal handling
; is a bit saner than 8bit.
;
#if 0
_intvector1:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt1
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector2:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt3
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector3:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt3
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector4:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt4
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector5:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt5
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector6:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt6
		movem.l (sp)+,a0-a6/d0-d7
		rte

_intvector7:	movem.l a0-a6/d0-d7,-(sp)
		jsr _interrupt7
		movem.l (sp)+,a0-a6/d0-d7
		rte
#endif
sig_or_die:	
		tst.b kernel_flag
		beq sig_user
		move.b (sp),d0
		jsr outcharhex
		lea panic_sig,a0
		jsr outstring
		lea panic_sig2,a0
		jsr panic			; won't return
panic_sig:	ascii ": signal in kernel mode."
		byte 13,10
panic_sig2:	asciz ": ksig"
		.align 2

sig_user:	movem.l a0-a6/d0-d7,-(sp)
		move.l (U_DATA__U_PTAB),-(sp)
		jsr ssig
		adda #4,sp
		; Do signal processing bit here if we return
		movem.l (sp)+,a0-a6/d0-d7
		rte

;
;	Debug
;
outstring:	move.b (a0)+,d0
		beq outstrend
		bsr outchar
		bra outstring
outstrend:	rts

outstringhex:	move.b (a0)+,d0
		beq outstrend
		bsr outcharhex
		bra outstringhex

hexdigit:
		add.b #48,d0
		cmp.b #58,d0
		bcs notltr
		add.b #7,d0
notltr:		bra outchar

outa0hex:	move.l a0,d0
		moveq #24,d1
		lsr.l d1,d0
		bsr outcharhex
		move.l a0,d0
		moveq #16,d1
		lsr.l d1,d0
		bsr outcharhex
		move.l a0,d0
		lsr.l #8,d0
		bsr outcharhex
		move.l a0,d0
outcharhex:	move.w d0,-(sp)
		lsr #4,d0
		bsr hexdigit
		move.w (sp),d0
		bsr hexdigit
		move.w (sp)+,d0
		rts

.area data
kernel_flag:	byte 0
