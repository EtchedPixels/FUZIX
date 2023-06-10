#include "../kernel-68000.def"


.globl plt_switchout,switchin,dofork,udata_shadow

.mri 1

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
plt_switchout:
        or #$0700,sr
        ; save machine state

	move.l usp,a0
	movem.l a0/a2-a4/a6/d2-d7,-(sp)
	move.l sp,U_DATA__U_SP(a5)	; this is where the SP is restored in switchin

        ; find another process to run (may select this one again)
        bsr getproc

	move.l d0,-(sp)
        bsr switchin

        ; we should never get here
        bra plt_monitor

switchin:
        or #$0700,sr
	move.l 4(sp),a0		; task to switch to
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
        ; check u_data->u_ptab matches what we wanted
	cmp.l U_DATA__U_PTAB(a5),a0
	bne switchinfail

	; Switch the stacks back
	move.l a5,udata_shadow	; For IRQs etc to recover A5 state
	move.l U_DATA__U_SP(a5),a7

	move.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)

	;
	;	Get the user pages back. We can safely
	;	do this above the parent kernel stack I think
	;
	clr.l -(sp)
	move.l a0,-(sp)
	jsr pagemap_switch
	addq #8,sp

        ; runticks = 0
	clr.w runticks

	; recover the task switch state
	movem.l (sp)+,a0/a2-a4/a6/d2-d7
	move.l a0,USP

        tst.b U_DATA__U_ININTERRUPT(a5)
        bne keepoff ; in ISR, leave interrupts off
	; Switch the IRQ mask - don't assume it will be 0, we mask some
	; interrupts all the time on some platforms.
	move.w sr,d0
        and #$F8FF,d0
	or.w #EI_MASK,d0
	move.w d0,sr
keepoff:
	clr.w d0
        rts ; return with interrupts on

switchinfail:
	bsr outa0hex
        lea badswitchmsg,a0
        bsr outstring
	; something went wrong and we didn't switch in what we asked for
        bra plt_monitor

	;
	; this gets exciting on the 68000 because our udata is not in a
	; fixed location except for swap only platforms. To deal with this
	; we return parent first and create a new return frame for the child
	;
	; Entry:
	; A5 = u_data pointer for parent
	; 4(sp) = child process table entry
	;
	; Exit:
	; We are running as the parent but the child frame is built
	;
dofork:
	move.l 4(sp),a0			;	child p_tab
	move.l a2,-(sp)

	move.l P_TAB__P_UDATA_OFFSET(a0),a1	; child udata

	; Copy the udata
	; FIXME: we don't need to copy the stack too!

	move.l a5,a2
	move.w #255,d0
loop:
	move.l (a2)+,(a1)+
	dbra d0,loop


	;
	; Save the child PID for later and the udata offset
	;
	move.w P_TAB__P_PID_OFFSET(a0),-(sp)
	move.l P_TAB__P_UDATA_OFFSET(a0),-(sp)
	;
	; Configure the child udata now we copied it

	move.l P_TAB__P_UDATA_OFFSET(a0),-(sp)
	move.l a0,-(sp)
	jsr makeproc
	addq #8,sp
	;
	;	The memory map is clone by ptab_alloc (we might want to
	;	adjust that bit)
	;
	;tst.l d0
	;bne forked_up

	move.l (sp)+,a2		; Get the udata pointer back
	;
	; Now build a custom return frame for the child
	;
	lea.l 1024(a2),a1	; End of stack
	;
	; Copy over the parent top of stack return info
	;
	;		68000		68010
	;   1022	PC.L		VECTOR
	;   1020	PC.H		PC.L
	;   1018	STATUS		PC.H
	;   1016	A5		STATUS
	;   1014	A5		A5
	;   1012			A5
	;
	tst.b cpu_has_trapvec
	beq cpu_3word

	move.w 1022(a5),-(a1)	; Copy the vector
	move.l 1018(a5),-(a1)	; Copy the PC
	move.w 1016(a5),-(a1)	; Copy the Status
	move.l 1012(a5),-(a1)	; Copy the A5 save of the parent
				; needed for PIC processes
	bra pushret
cpu_3word:
	move.l 1020(a5),-(a1)	; Copy the PC
	move.w 1018(a5),-(a1)	; Copy the Status
	move.l 1014(a5),-(a1)	; Copy A5
pushret:
	;
	; Stack a return address
	;
	move.l #forkreturn,-(a1)

	;
	; USP has to be in a0
	;
	move.l usp,a0
	;
	; And a frame as if we did a switchout
	;
	movem.l a0/a2-a4/a6/d2-d7,-(a1)
	move.l a1,U_DATA__U_SP(a2)	; Set the stack pointer
	clr.b U_DATA__U_INSYS(a2)	; set child not in a syscall
	; And carry on as the parent
	move.w (sp)+,d0		; child pid
	move.l (sp)+,a2
	clr.w runticks
	rts
forked_up:
	moveq #-1,d0			;	parent, failed
	rts
;
;	This code gets called when we resume the child
;
forkreturn:
	; Wipe any stray kernel data
	moveq #0,d0
	moveq #0,d1
	moveq #0,d2
	moveq #0,d3
	moveq #0,d4
	moveq #0,d5
	moveq #0,d6
	moveq #0,d7
	move.l d0,a0
	move.l d0,a1
	move.l d0,a2
	move.l d0,a3
	move.l d0,a4
	move.l d0,a6
	; recover A5 to match the parent
	move.l (sp)+,a5
	; and RTE to the same address (which is fine as the user memory
	; has already been juggled if needed).
	rte

badswitchmsg: ascii "_switchin: FAIL"
            byte 13,10,0
.even
