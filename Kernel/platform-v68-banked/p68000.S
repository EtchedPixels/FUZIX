
#include "../kernel-68000.def"
/*
 *	Lots left to fill in
 */

		.globl trap_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl trap_monitor
		.globl udata_block
		.globl devide_read_data
		.globl devide_write_data
		.globl vdso
.mri 1
trap_reboot:
trap_monitor:
	    or #0700,sr
	    bra trap_monitor

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
	    move.w #13,d0
trapset:
	    move.l #misctrap,(a0)+
	    dbra d0,trapset
	    move.l #trap14,(a0)+
	    move.l #trap15,(a0)+
	    move.w #$0,a0
	    move.l #uninit,$3c(a0)
	    move.l #spurious,$60(a0)
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
	    bra trap_via_signal		; a0/a1/d0/d1 already stacked

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
;	Switch the banking hardware.
;
map_process_always:
	; make sure the current process is mapped.
	move.b U_DATA__U_PAGEB(a5),$00F05000
	rts
map_process:
	; map a specific process (not actually used)
	move.b P_TAB__P_PAGE_OFFSETB(a5),$00F05000
	rts
map_restore:
	move.b $00F05000,banksave
	rts
map_save:
	move.b banksave,$00F05000
	rts
map_kernel:				; we never bank out the kernel
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
;	IDE. This differs a tiny bit from the simple case because we must
;	support bank switching while handling swap.
;
devide_read_data:
	lea.l blk_op,a0
	move.b 4(a0),d0
	move.b $00F05000,d1
	cmp.b #2,d0
	bne notswap
	move.b 5(a0),$00F05000
notswap:
	move.w #255,d0
	move.l (a0),a0
devide_read_l:
	move.w $00F01000,(a0)+
	dbra d0,devide_read_l
	move.b d1,$00F05000
	rts

devide_write_data:
	lea.l blk_op,a0
	move.b 4(a0),d0
	move.b $00F05000,d1
	cmp.b #2,d0
	bne notswapw
	move.b 5(a0),$00F05000
notswapw:
	move.w #255,d0
	move.l (a0),a0
devide_write_l:
	move.w (a0)+,$00F01000
	dbra d0,devide_write_l
	move.b d1,$00F05000
	rts

;
;	'VDSO'
;
vdso:	trap #14		; syscall entry
	rts
	; signal unwind
	movem.l (sp)+,a0/a1/d0/d1
	move.w (sp)+,ccr
	rts
	; rest is spare for now

.section data

kernel_flag: byte 1
banksave:    byte 0
