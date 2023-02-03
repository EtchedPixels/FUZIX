
#include "../kernel-68000.def"
#include "config.h"

#ifdef MINI68K_TRAP_BIOSCALL
#include "biostrap.def"
#include "ns202.h"
#endif

	.globl __builtin_bswap32, __builtin_bswap16
	.globl plt_reboot
	.globl init_early
	.globl init_hardware
	.globl program_vectors
	.globl plt_idle
	.globl outchar
	.globl OUTCHAR		/* C-callable */
	.globl plt_monitor
	.globl plt_daytime_setup
	.globl udata_block
	.globl irq0,irq1,irq2,irq3,irq4,irq5,irq6,irq7
	.globl irq8,irq9,irq10,irq11,irq12,irq13,irq14,irq15
	.globl timer_irq	/* same as 'irq8' */
	.globl tod		/* Time of Day */
	.globl __muldi3
		
	.globl break_here	/* strictly for Debugging */

/* define the RETI instruction to signal end-of-interrupt to the ns32202 */
#if (eoi+IOBASE == 0xFFFFA040)
#define RETI $FFFFA040	/* bit 13, device 40, register 0 (hvct) */
#else
/*#error "cannot define RETI instuction for ns202" */
#endif

LSR = IOBASE+mf_sio+5			/* 0x48 + 5 */
THRE = 5				/* bit 5 */
THR = IOBASE+mf_sio+0			/* 0x48 + 0 */
RBR = IOBASE+mf_sio+0			/* 0x48 + 0 */

.mri 1
plt_monitor:
plt_reboot:
	or #DI_MASK,sr
	bra plt_monitor

init_early:
	move.w	#0,a5
	move.l	(a5),cmdline		; init the 'cmdline' char pointer
	
	lea.l udata_block,a5	; udata ptr
	move.l a5,udata_shadow	; shadow copy for entry/exit
	    
;;;	    bsr uart_scan	/* Mini-M68k only */
	    
        rts

plt_idle:
	stop #($2000+EI_MASK)
	rts
;
;	FIXME: could be in discard if we wanted
;
init_hardware:
	clr.l	d1			; alloc zero bytes
	movql	#hma_alloc,d0		; BIOS call to find Highest Memory Address
	trap	#BIOS
	lsr.l	#8,d0			; convert bytes to kilobytes
	lsr.l	#2,d0			; shift is by a total of 10
;;;kernelmem:
	move.w d0,ramsize
	move.l #_end+512,d1		; End of kernel - to round off
	lsr.l #8,d1
	lsr.l #2,d1
	sub.w  d1,d0			; Top of memory minus kernel 
	move.w d0,procmem		; Save for report

	lea	tod,a0
	move.l	a0,-(sp)
	jsr	plt_daytime_setup	; get time from dead BIOS clock
	lea	4(sp),sp

#define TRAP8 40*4		/* (32+8)*4 */
#define FMT14 14*4		/* Format vector on 68010 */

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/* vector saves */
#if MINI_M68K_VSAVE & 0
	move.w	#10-1,d0
	lea	12*4,a0	; vector 12 is one beyond what we want to save
vsave:
	move.l	-(a0),-(sp)	; stack starting at vector 11
	dbra.w	d0,vsave
#if CPU>=68010
	move.l	FMT14,-(sp)	; Format exception on 68010
#endif
#endif
	move.l	TRAP8,-(sp)
; end save

	bsr install_vectors

/* vector restores */
	move.l	(sp)+,TRAP8		; save the Mini BIOS call trap
#if MINI_M68K_VSAVE & 0
#if CPU>=68010
	move.l	(sp)+,FMT14
#endif
	move.w	#10-1,d0
	lea	2*4,a0
vrestore:
	move.l	(sp)+,(a0)+
	dbra.w	d0,vrestore
#endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;	.globl break_here for testing
break_here:
/* install interrupt vectors	*/
	lea	64*4,a0			; start at vector 64
	move.l #irq0,(a0)+
	move.l #irq1,(a0)+
	move.l #irq2,(a0)+
	move.l #irq3,(a0)+
	move.l #irq4,(a0)+
	move.l #irq5,(a0)+
	move.l #irq6,(a0)+
	move.l #irq7,(a0)+
	move.l #irq8,(a0)+
	move.l #irq9,(a0)+
	move.l #irq10,(a0)+
	move.l #irq11,(a0)+
	move.l #irq12,(a0)+		/* MF/PIC (console) UART interrupt */
	move.l #irq13,(a0)+
	move.l #irq14,(a0)+
	move.l #irq15,(a0)

	bsr ns202_init		; initialize the interrupt system
#if DEBUG|1
	move.b	#$AA,(-1)	/* to the lites */
#endif
	jmp    c_init_hardware	/* to C init code */


/* define the RETI instruction to signal end-of-interrupt to the ns32202 */
#if (eoi+IOBASE == 0xFFFFA040)
#define RETI $FFFFA040	/* bit 13, device 40, register 0 (hvct) */
#else
/*#error "cannot define RETI instuction for ns202" */
#endif





/* ****** Fast Timer -- timerH on the ns32202 chip ******* */

#if defined(INT_TIMER) & (INT_TIMER==8)
irq8:
;;;	and.w	#$F8FF,sr		; re-enable all vectored interrups
					; the NS202 handles the masking of
					; those of lower priority
;;;	or.w	#EI_MASK,sr		; something to make stuff portable ???

/****  Debuugging indicates that ALL interrupt routines must run LOCKED *****/

timer_irq:
	; C will save and restore a2+/d2+
	movem.l a0-a1/a5/d0-d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	move.b #1,U_DATA__U_ININTERRUPT(a5)
	    
	bsr  plt_interrupt		/* call platform interrupt */
	    
	clr.b U_DATA__U_ININTERRUPT(a5)

	tst.b U_DATA__U_INSYS(a5)
	bne no_preempt
	tst.b need_resched
	beq no_preempt
	;
	;	Vanish into the scheduler. Some other task will pop back out
	;   and eventually we'll re-appear here and continue.
	;
	;   FIXME: check IRQ masking
	;
	move.l U_DATA__U_PTAB(a5),a0
	; We might be in STOPPED state in which case do not go back
	; to ready !
	cmp.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	bne no_ready
	move.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
	ori.b #PFL_BATCH,P_TAB__P_FLAGS_OFFSET(a0)
no_ready:
	bsr switchout
no_preempt:
	tst.b U_DATA__U_CURSIG(a5)
	beq no_signal
	;   FIXME: this is ugly
	movem.l (sp)+,a0-a1/a5/d0-d1
	move.w sr,-(sp)
	move.w #0,-(sp)
	movem.l a0-a6/d0-d7,-(sp)
	move.l (sp),-(sp)
	bsr exception
	addq #4,sp
	or.w #DI_MASK,sr		; disable all interrupts	
	move.b RETI,d0		; NS202  RETI  return from interrupt
	movem.l (sp)+,a0-a6/d0-d7
	addq #4,sp
	rte				; re-enable interrupts

no_signal:
	or.w	#DI_MASK,sr		; disable all interrupts	
	move.b	RETI,d0			; NS202  RETI  return from interrupt
	movem.l (sp)+,a0-a1/a5/d0-d1
	rte				; re-enable interrupts
#endif	

;
;	Nothing to do in 68000 - all set up once at boot
;
program_vectors:
	rts

;
;	We do no banking so we need to do nothing here.
;
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	rts

; outchar: Wait for UART TX idle, then print the char in d0

OUTCHAR:			/* C-callable */
	move.b	7(sp),d0
outchar:
outcharw:
	btst #THRE,LSR			/* check LSR.THRE */
	beq outcharw
	move.b d0,THR			/* out to THR */
	rts

/* start by handling all the ns202 interrupts */
irq0:
irq1:
irq2:
irq3:
irq4:
irq5:
irq6:
irq7:
#if !defined(INT_TIMER) | !(INT_TIMER==8)	/* catch re-definition error */
irq8:
#endif
irq9:
irq10:
irq11:
#if !defined(INT_TTY1) | !(INT_TTY1==12)
irq12:
#endif
irq13:
irq14:
irq15:
	and.w	#$F8FF,sr		; re-enable all vectored interrups
					; the NS202 handles the masking of
					; those of lower priority
	or.w	#EI_MASK,sr		; something to make stuff portable ???	
	movem.l	d0-d1/a0-a1,-(sp)	; save regs not saved by C-programs
	
	; bsr  C-callable interrupt handler

	or.w	#DI_MASK,sr		; disable all interrupts	
	move.b	RETI,d0			; NS202  RETI  return from interrupt
	movem.l	(sp)+,d0-d1/a0-a1	; restore regs not saved by C-programs
	rte				; restore SR and return to processing
	
/* ****** Console (MF/PIC) UART interrupt ***** */

#if defined(INT_TTY1) & (INT_TTY1==12)
irq12:

/****  Debuugging indicates that ALL interrupt routines must run LOCKED *****/

;;;	and.w	#$F8FF,sr		; re-enable all vectored interrups
					; the NS202 handles the masking of
					; those of lower priority

;;;	or.w	#EI_MASK,sr		; something to make stuff portable ???

	movem.l	d0-d1/a0-a1/a5,-(sp)	; save regs not saved by C-programs
	move.l udata_shadow,a5	; set up the register global
	move.b #1,U_DATA__U_ININTERRUPT(a5)

	; bsr  C-callable interrupt handler
	jbsr	tty1_uart_interrupt

	clr.b U_DATA__U_ININTERRUPT(a5)
	or.w	#DI_MASK,sr		; disable all interrupts	
	move.b	RETI,d0			; NS202  RETI  return from interrupt
	movem.l	(sp)+,d0-d1/a0-a1/a5	; restore regs not saved by C-programs
	rte				; restore SR and return to processing
#endif	

__muldi3:				; longlong = longlong * longlong
	movm.l	d2-d7/a2,-(sp)
	move.l	$24(sp),a0
	move.l	$2c(sp),d5
	move.l	a0,d0
	move.l	d5,d1
	move.l	d0,d2
	swap	d0
	move.l	d1,d3
	swap	d1
	move.w	d2,d4
	mulu	d3,d4
	mulu	d1,d2
	mulu	d0,d3
	mulu	d0,d1
	move.l	d4,d0
	eor.w	d0,d0
	swap	d0
	add.l	d0,d2
	add.l	d3,d2
	bcc.s	Lafc4
	add.l	#$10000,d1
Lafc4:	swap	d2
	movq.l	#0,d0
	move.w	d2,d0
	move.w	d4,d2
	move.l	d2,d6
	add.l	d1,d0
	move.l	d0,d7
	lea	__mulsi3,a2
	move.l	$28(sp),-(sp)
	move.l	a0,-(sp)
	jsr	(a2)
	addq.l	#8,sp
	add.l	d0,d7
	move.l	d5,-(sp)
	move.l	$24(sp),-(sp)
	jsr	(a2)
	addq.l	#8,sp
	add.l	d7,d0
	move.l	d6,d1
	movm.l	(sp)+,d2-d7/a2
	rts

__builtin_bswap32:
	move.l	4(sp),d0
	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0
	rts

__builtin_bswap16:
	clr.l	d0
	move.w	6(sp),d0
	ror.w	#8,d0
	rts
	
