
#include "../kernel-68000.def"

		.globl platform_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl platform_monitor
		.globl udata_block0
		.globl devide_read_data
		.globl devide_write_data

.mri 1
platform_monitor:
platform_reboot:
;
;	FIXME: remap ROM at 0 and get back to it
;
	    or #$0700,sr
	    bra platform_monitor

init_early:
	    lea.l udata_block0,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

;
;	FIXME: could be in discard if we wanted
;
init_hardware:
            ; set system RAM size(hardcode hacks for now)
	    move.w #768,d0		; ignore the ramdisc
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    bsr install_vectors

	    /* We use Vector 0x81 for the DUART */
	    move.l #duart_irq,$204
	    /* and the timer.. */
	    /* 0x89 is the 68302 10ms timer */
	    move.l #timer_irq,$224

            rts

duart_irq:
	    movem.l a0/a1/a5/d0/d1,-(sp)
	    jsr duart_interrupt
	    move.w #2,$FFF818		; clear interrupt flag
	    movem.l (sp)+,a0/a1/a5/d0/d1
	    rte

timer_irq:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    ; Now ack the hardware interrupt
	    move.b #3,$FFF849		; Timer A event register
	    move.w #$200,$FFF818	; Interrupt service
	    jsr platform_interrupt
	    clr.b U_DATA__U_ININTERRUPT(a5)

	    tst.b U_DATA__U_INSYS(a5)
	    bne no_preempt
	    tst.b need_resched
	    beq no_preempt
	    ;
	    ;	Vanish into the scheduler. Some other task will pop back out
	    ;   and eventually we'll re-appear here and continue.
	    ;
	    move.l U_DATA__U_PTAB(a5),a0
	    ; We might be in STOPPED state in which case do not go back
	    ; to ready !
	    cmp.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	    bne no_ready
	    move.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
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
	    movem.l (sp)+,a0-a6/d0-d7
	    addq #4,sp
	    rte
no_signal:
	    movem.l (sp)+,a0-a1/a5/d0-d1
	    rte

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

outchar:
outcharw:
	btst #2,$00C00002
	beq outcharw
	move.b d0,$00C00006
	rts

;
;	ATA CF support via PPIDE
;

#define PortA8255	$300001
#define PortB8255	$300003
#define PortC8255	$300005
#define Port8255Ctrl	$300007

;
;	Need to look at the fixes on write for the V1 board reliability.
;
devide_read_data:
	movem.l a2/d2/d3,-(sp)
	; Target address
	move.l (blk_op),a0
	; Set up constants for speed
	lea.l PortC8255,a1
	lea.l PortA8255,a2
	move.b #$40,d1
	move.b #$48,d2
	; 256 words
	; Just in case a user is trying to do a misaligned raw I/O
	move.w a0,d0
	btst #0,d0
	bne doreadblock8
	move.w #255,d0
readblock:
	; Waggle the control signals
	move.b d1,(a1)
	move.b d2,(a1)
	; 16bits split across two words
	movep.w 0(a2),d3
	; Store to the target buffer
	move.w d3,(a0)+
	dbra d0,readblock
	move.b d1,(a1)
	movem.l (sp)+,a2/d2/d3
	rts

doreadblock8:
	move.l a3,-(sp)
	lea.l PortB8255,a3
	move.w #255,d0
readblock8:
	; Waggle the control signals
	move.b d1,(a1)
	move.b d2,(a1)
	; 16bits split across two words
	move.b (a3),(a0)+
	move.b (a2),(a0)+
	dbra d0,readblock8
	move.b d1,(a1)
	move.l (sp)+,a3
	movem.l (sp)+,a2/d2/d3
	rts

; FIXME: can we use movep

devide_write_data:
	movem.l a2-a3/d2,-(sp)
	move.l (blk_op),a0
	lea.l PortA8255,a1
	lea.l PortB8255,a2
	lea.l PortC8255,a3
	move.b #$40,d1
	move.b #$50,d2
	move.b #$80,Port8255Ctrl
	move.w #255,d0
writeblock:
	move.b (a0)+,(a1)
	move.b (a0)+,(a2)
	move.b d1,(a3)
	move.b d2,(a3)
	move.b d1,(a3)
	dbra d0,writeblock
	move.b #$92,Port8255Ctrl
	movem.l (sp)+,a2-a3/d2
	rts
