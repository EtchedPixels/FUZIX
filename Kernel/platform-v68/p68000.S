
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
.mri 1
platform_reboot:
platform_monitor:
	    or #0700,sr
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
	    move.w #512,d0
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    bsr install_vectors

	    move.w #$0,a0

	    move.l #timer_irq,$78(a0)
	    move.l #mmu_fault,$7C(a0)

	    moveq #1,d0			;
	    move.b d0,$00F04000
            rts

timer_irq:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr platform_interrupt
	    clr.b U_DATA__U_ININTERRUPT(a5)
	    move.b $00F04000,d0		; Re-enable

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
	    ; We might be in STOPPED state in which case do not go back
	    ; to ready !
	    move.l U_DATA__U_PTAB(a5),a0
	    cmp.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	    bne no_ready
	    move.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
	    ori.b #PFL_BATCH,P_TAB__P_FLAGS_OFFSET(a0)
no_ready:
	    move.l U_DATA__U_PTAB(a5),a0
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

mmu_fault:
	    tst.w $00F02020		; clear the MMU flag
	    move.l d0,-(sp)
	    move.b #'M',d0
	    jsr outchar
	    move.l (sp)+,d0
	    bra addr_error		; treat as an address error

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
	    move.w d0,-(sp)
outcharw:
	    move.b $00F03010,d0
	    btst #1,d0
	    beq outcharw
	    move.w (sp)+,d0
	    move.b d0,$00F03000
	    rts

;
;	IDE
;
devide_read_data:
	move.l blk_op,a0
	move.w #255,d0
devide_read_l:
	move.w $00F01000,(a0)+
	dbra d0,devide_read_l
	rts

devide_write_data:
	move.l blk_op,a0
	move.w #255,d0
devide_write_l:
	move.w (a0)+,$00F01000
	dbra d0,devide_write_l
	rts

