#include "../kernel-68000.def"
#include "kernel.def"


.globl switchout,switchin,dofork,udata_shadow,udata_stash

.mri 1

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
switchout:
        or #$0700,sr
        bsr chksigs
        ; save machine state

        clr.w -(sp) ; return code set here is ignored, but switchin can
        ; return from either switchout OR dofork, so they must both write
        ; U_DATA__U_SP with the following on the stack:
	move.l usp,a0
	movem.l a0/a2-a4/a6/d2-d7,-(sp)
	move.l sp,U_DATA__U_SP(a5)	; this is where the SP is restored in switchin
	;
	; keep a fast path for wakeup of the last executed process
	;
	tst.b nready
	bne slow_path
idling:
        and #$F8FF,sr
	jsr platform_idle
        or #$0700,sr
	move.b nready,d0
	beq idling
	cmp.b #1,d0
	bne slow_path
	move.l U_DATA__U_PTAB(a5),a0
	cmp.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
	bne slow_path
	mov.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	movem.l (sp)+,a0/a2-a4/a6/d2-d7
	addq #2,sp
        and #$F8FF,sr
	rts

slow_path:
	lea.l udata_stash,a0
	move.l a5,a1
	move.w #255,d0
	; FIXME - optimise 1K copy
copyi:	move.l (a1)+,(a0)+
	dbra d0,copyi

        ; find another process to run (may select this one again)
        bsr getproc

	move.l d0,-(sp)
        bsr switchin

        ; we should never get here
        bra trap_monitor

switchin:
        or #$0700,sr
	move.l 4(sp),a0		; task to switch to

	;
	;	We are going to copy over the process stack so use the
	;	other stack for this
	;
	lea.l irqstack+256,sp	; switch stack to copy the udata/stack

	;
	;	Now switch task context
	;
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
	tst.w P_TAB__P_PAGE_OFFSET(a0)	; swapped or existing process ?
	bne not_swapped

;
;	FIXME: sort IRQ enables
;

	move.l a0,-(sp)
	move.l a0,-(sp)
	jsr swapper
	addq #4,sp
	move.l (sp)+,a0

        or #$0700,sr

not_swapped:
	move.w P_TAB__P_PAGE_OFFSET(a0),d0
	; switch memory bank
	move.b d0,$FFF05000
	; is the cached udata still valid ?
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
	cmp.l U_DATA__U_PTAB(a5),a0
	beq skip_copyback
	;
	;	Recover the udata block
	;
	lea.l udata_stash,a1
	move.w #255,d0
copyo:
	move.l (a1)+,(a5)+
	dbra d0,copyo

skip_copyback:
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
        ; check u_data->u_ptab matches what we wanted
	cmp.l U_DATA__U_PTAB(a5),a0
	bne switchinfail

	move.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	move.w P_TAB__P_PAGE_OFFSET(a0),U_DATA__U_PAGE(a5)

        ; runticks = 0
	clr.w runticks

        ; restore machine state
        move.l U_DATA__U_SP(a5),sp
	movem.l (sp)+,a0/a2-a4/a6/d2-d7
	move.l a0,usp
	move.w (sp)+,d0			; FIXME: can we merge ?

        tst.b U_DATA__U_ININTERRUPT(a5)
        bne keepoff ; in ISR, leave interrupts off
        and #$F8FF,sr
keepoff:
        rts ; return with interrupts on

switchinfail:
	bsr outa0hex
        lea badswitchmsg,a0
        bsr outstring
	; something went wrong and we didn't switch in what we asked for
        bra trap_monitor

	;
	; this gets exciting on the 68000 because our udata is not in a
	; fixed location except for swap only platforms. That means any
	; udata relative pointers on the stack when we duplicate the kernel
	; stack point to the parent. For the simple case we have a single
	; swapped udata and stack so all is fairly easy. For the other
	; cases we have to return as the parent but set up a fake stack
	; frame for the child to exit the syscall. Simply being careful
	; about pointers doesn't work - the compiler will internally
	; use link/unlk and other stuff.
	;
	; Entry:
	; A5 = u_data pointer for parent
	; 4(sp) = child process table entry
	;
	; Exit:
	; We are running as the child, A5 = u_data pointer of child, on
	; child stack and leap directly back to user mode
	;
dofork:
	;
	; We need the child to return off its own stack so this is very
	; different to other platforms.
	;
	move.l 4(sp),a0			;	child p_tab
	;
	; in the simple case we only have one udata. In the complex cases
	; we would have to compute the new one and load it into a5 and
	; offset

	;
	; Set up a switchin frame for the parent process
	;
	move.w P_TAB__P_PID_OFFSET(a0),-(sp)	;	child pid (parent return)
	move.l usp,a0
	movem.l a0/a2-a4/a6/d2-d7,-(sp)	;	save state
	move.l sp,U_DATA__U_SP(a5)	;	save pointer

	move.l a0,-(sp)			;	argument to newproc
	move.w P_TAB__P_PAGE_OFFSET(a0),d0	; page

	jsr bankfork			;	copy the memory

	move.l (sp),a0
	
	jsr newproc			;	Called with the child udata
	add.w #54,sp			;	toss the stack frames

	clr.w runticks
	moveq #0,d0			;	child
	rts

bankfork:
	; copy the memory banks over (we copy the lot we should fast copy
	; the actual used low/high chunks and then zero the middle
	;
	; FIXME: write it to run at proper speed etc
	;
	; Caution we are copying our own stack here
	;
	;
	move.l d2,-(sp)
	move.l #$02000000,a0		;	process base
	move.b $FFF05000,d1		;	parent page

copyloop:
	move.b d1,$FFF05000		;	parent bank
	move.l (a0),d0
	move.b d2,$FFF05000		;	child bank
	move.l d0,(a0)+
	cmp.l #$03000000,a0
	bne copyloop
	; Return with child bank paged as we want it
	move.l (sp)+,d2			;	we swapped stack but copied d2
	rts				;	so popping off the other bank
					;	is fine

badswitchmsg: ascii "_switchin: FAIL"
            byte 13,10,0
.even


udata_stash	equ	UDATA_STASH
