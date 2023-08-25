
#include "../kernel-68000.def"

#define SBUF	$80002021
#define SCON	$80002023
#define RCAPH0	$80002032
#define T0CON	$80002035
#define T0IR	$80002037

#define T100HZ	$E525


		.globl plt_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl plt_idle
		.globl outchar
		.globl plt_monitor
		.globl udata_block
		.globl devide_read_data
		.globl devide_write_data

.mri 1
plt_monitor:
	; Need to check if top of memory at FE7000, if so
	; copy vectors from FE7000 to 0 (where we saved them)
	; them jmp to FE8000
plt_reboot:
	    or #$0700,sr
	    bra plt_monitor

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

plt_idle:
	    stop #($2000+EI_MASK)
	    rts
;
;	FIXME: could be in discard if we wanted
;
init_hardware:
	    move.w #512,d0
	    move.w d0,ramsize
	    move.l #_end+512,d1
	    lsr.l #8,d1
	    lsr.l #2,d1
	    sub.w  d1,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    bsr install_vectors
	    move.l #timer_irq,$6C	; autovector 3
	    move.l #serial_irq,$70	; autovector 4

	    move.w #T100HZ,RCAPH0	; 100Hz tick
	    move.b #4,T0CON
	    move.b #3,T0IR		; Interrupt autovector 3
            rts

timer_irq:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr plt_interrupt
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
	    movem.l (sp)+,a0-a6/d0-d7
	    addq #4,sp
	    rte
no_signal:
	    movem.l (sp)+,a0-a1/a5/d0-d1
	    rte

serial_irq:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr tty_interrupt
	    clr.b U_DATA__U_ININTERRUPT(a5)
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
	btst #2,SCON
	beq outcharw
	move.b d0,SBUF
	rts
;
;	IDE:
;
devide_read_data:
	move.l blk_op,a0
	move.l #$12000000,a1
	move.w #255,d0
devide_read_l:
	move.b (a1),(a0)+
	move.b (a1),(a0)+
	dbra d0,devide_read_l
	rts

devide_write_data:
	move.l blk_op,a0
	move.l #$12000000,a1
	move.w #255,d0
devide_write_l:
	move.b (a0)+,(a1)
	move.b (a0)+,(a1)
	dbra d0,devide_write_l
	rts
