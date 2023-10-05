
#include "../../cpu-68000/kernel-68000.def"

		.globl plt_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl interrupt
		.globl plt_idle
		.globl outchar
		.globl plt_monitor

		.globl timer_tick
		.globl rc2014_vector

		.globl devide_read_data
		.globl devide_write_data

.mri 1
plt_monitor:
;
;	The address latch is one way except for 'reset' which flips it
;	back. Write a reset into our RAM (now unused) two bytes before
;	the actual entry point in the other bank. When we reset the
;	memory will flip back, the reset will clear the prefetch queue
;	and we will begin executing as if we booted the ROM
;
plt_reboot:
	    move.l $04000000,a7		; stack expected by boot ROM
	    move.l $04000004,a0		; start address in boot ROM
	    ori.w #$2700,sr
	    move.l copyme,-2(a0)
	    jmp -2(a0)
copyme:
	    reset

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

plt_idle:
	    stop #($2000+EI_MASK)
	    rts
init_hardware:
	    move.w #16384,d0
	    move.w d0,ramsize
	    move.l #_end+512,d1
	    sub.l #_data,d1
	    lsr.l #8,d1
	    lsr.l #2,d1
	    sub.w  d1,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    jmp c_init_hardware

rc2014_vector:
	    movem.l a0-a1/a5/d0-d2,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b U_DATA__U_ININTERRUPT(a5),d2
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr rc2014_interrupt
	    move.b d2,U_DATA__U_ININTERRUPT(a5)
	    movem.l (sp)+,a0-a1/a5/d0-d2
	    rte

timer_tick:
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

;
;	Nothing to do in 68000 - all set up once at link time
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

;	IDE:
;
devide_read_data:
	move.l	blk_op,a0
	move.w	#$F010,a1
	move.w	#511,d0
devide_read_l:
	move.b	(a1),(a0)+
	dbra	d0,devide_read_l
	rts

devide_write_data:
	move.l	blk_op,a0
	move.w	#$F010,a1
	move.w	#511,d0
devide_write_l:
	move.b	(a0)+,(a1)
	dbra	d0,devide_write_l
	rts

; outchar: Wait for UART TX idle, then print the char in d0

outchar:
outcharw:
	; TODO
	rts
;
