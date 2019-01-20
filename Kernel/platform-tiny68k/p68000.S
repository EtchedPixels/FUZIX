
#include "../kernel-68000.def"
/*
 *	Lots left to fill in
 */

		.globl platform_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl platform_monitor
		.globl udata_block
		.globl devide_read_data
		.globl devide_write_data
		.globl vdso

#define CTUR $D
#define CTLR $F
#define IMR  $B
#define IVR $19

.mri 1
platform_monitor:
	; Need to check if top of memory at FE7000, if so
	; copy vectors from FE7000 to 0 (where we saved them)
	; them jmp to FE8000
platform_reboot:
	    or #$0700,sr
	    bra platform_monitor

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

;
;	FIXME: could be in discard if we wanted
;
init_hardware:
            ; set system RAM size(hardcode hacks for now)
	    move.w #16352,d0
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    move.l #8,a0
	    move.w #253,d0
	    move.l #unexpected,d1
init_trap_loop:
	    move.l d1,(a0)+
	    dbra d0,init_trap_loop
	    ;
	    ; Now set the vectors we care about
	    ;
	    move.w #8,a0
	    move.l #bus_error,(a0)+
	    move.l #addr_error,(a0)+
	    move.l #illegal,(a0)+
	    move.l #divzero,(a0)+
	    move.l #chk,(a0)+
	    move.l #trapv,(a0)+
	    move.l #priv,(a0)+
	    move.l #trace,(a0)+
	    move.l #unimpa,(a0)+	; A and F line traps
	    move.l #unimpf,(a0)+
	    move.w #$80,a0
	    move.l #trap0,(a0)+
	    move.l #trap1,(a0)+
	    move.l #trap2,(a0)+
	    move.l #trap3,(a0)+
	    move.l #trap4,(a0)+
	    move.l #trap5,(a0)+
	    move.l #trap6,(a0)+
	    move.l #trap7,(a0)+
	    move.l #trap8,(a0)+
	    move.l #trap9,(a0)+
	    move.l #trap10,(a0)+
	    move.l #trap11,(a0)+
	    move.l #trap12,(a0)+
	    move.l #trap13,(a0)+
	    move.l #trap14,(a0)+
	    move.l #trap15,(a0)
	    move.w #$0,a0
	    move.l #uninit,$3c(a0)
	    move.l #spurious,$60(a0)
	    /* We use Vector 0x81 for the DUART */
	    move.l #timer_irq,$204(a0)

	    lea.l $FFF000,a0
	    move.b #$80,CTLR(a0)
	    move.b #$4,CTUR(a0)
	    move.b #$81,IVR(a0)
	    move.b #$2A,IMR(a0)
            rts

timer_irq:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr platform_interrupt
	    clr.b U_DATA__U_ININTERRUPT(a5)

	    tst.b U_DATA__U_INSYS(a5)
	    bne no_preempt
	    tst.b need_resched
	    bne no_preempt
	    ;
	    ;	Vanish into the scheduler. Some other task will pop back out
	    ;   and eventually we'll re-appear here and continue.
	    ;
	    ;   FIXME: check IRQ masking
	    ;
	    move.l U_DATA__U_PTAB(a5),a0
	    move.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
	    bsr switchout
no_preempt:
	    tst.b U_DATA__U_CURSIG(a5)
	    beq no_signal
	    ;   FIXME: this is ugly
	    movem.l (sp)+,a0-a1/a5/d0-d1
	    move.w sr,-(sp)
	    move.w #0,-(sp)
	    movem.l a0-a6/d0-d7,-(sp)
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
	btst #2,$00FFF003
	beq outcharw
	move.b d0,$00FFF007
	rts
;
;	IDE:
;
devide_read_data:
	move.l blk_op,a0
	move.l #$00FFE000,a1
	move.w #255,d0
devide_read_l:
	move.w (a1),(a0)+
	dbra d0,devide_read_l
	rts

devide_write_data:
	move.l blk_op,a0
	move.l #$00FFE000,a1
	move.w #255,d0
devide_write_l:
	move.w (a0)+,(a1)
	dbra d0,devide_write_l
	rts

;
;	'VDSO'
;
vdso:	trap #14		; syscall entry
	rts
	; signal unwind
	move.l 8(sp),sp		; blow away stack frame
	movem.l (sp)+,a0/a1/d0/d1
	move.w (sp)+,ccr
	rts
	; rest is spare for now

