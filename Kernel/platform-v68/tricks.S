#include "../kernel-68000.def"


.globl platform_switchout,switchin,dofork,udata_shadow

.mri 1

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
platform_switchout:
        or #$0700,sr
        ; save machine state

        clr.w -(sp) ; return code set here is ignored, but switchin can
        ; return from either switchout OR dofork, so they must both write
        ; U_DATA__U_SP with the following on the stack:
	move.l usp,a0
	movem.l a0/a2-a4/a6/d2-d7,-(sp)
	move.l sp,U_DATA__U_SP(a5)	; this is where the SP is restored in switchin

        ; find another process to run (may select this one again)
        bsr getproc

	move.l d0,-(sp)
        bsr switchin

        ; we should never get here
        bra platform_monitor

switchin:
        or #$0700,sr
	move.l 4(sp),a0		; task to switch to
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
	tst.w P_TAB__P_PAGE_OFFSET(a0)	; swapped or existing process ?
	bne not_swapped

;
;	If you have one udata and swap them then you need to stack switch
;	for the swap in as you'll overwrite the kernel stack. You can use
; 	the IRQ stack but only if you leave IRQs off for a swap in (ugghh)
;

;
;	FIXME: sort IRQ enables
;

;
;	In simple mode the existing process always gets the boot here
;	before we can swap the new one in
;
	move.l U_DATA__U_PTAB(a5),a1	; old process
	tst.w P_TAB__P_PAGE_OFFSET(a1)	; swapped out/dead ?
	beq its_dead_jim		; corpses don't need swapping out

	move.l a0,-(sp)
	move.l a1,-(sp)
	jsr swapout
	addq #4,sp
	move.l (sp)+,a0

its_dead_jim:
	move.l sp,a1
	move.l #irqstack+256,sp
	move.l a1,-(sp)
	move.l a0,-(sp)
	move.l a0,-(sp)
	jsr swapper
	addq #4,sp
	move.l (sp)+,a0
	move.w #1,P_TAB__P_PAGE_OFFSET(a0) ; swapped in
	move.l (sp)+,a1			; straight into a7 fails
	move.l a1,a7			; emulator bug or CPU funny ??

        or #$0700,sr

not_swapped:
        ; check u_data->u_ptab matches what we wanted
	cmp.l U_DATA__U_PTAB(a5),a0
	bne switchinfail

	move.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)

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
        bra platform_monitor

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
	; child stack
	;
dofork:
	;
	; simple edition for swap only
	;
	move.l 4(sp),a0			;	child p_tab
	;
	; in the simple case we only have one udata. In the complex cases
	; we would have to compute the new one and load it into a5 and
	; offset
	;
	move.l a5,P_TAB__P_UDATA_OFFSET(a0)
	move.w P_TAB__P_PID_OFFSET(a0),-(sp)	;	child pid (parent return)
	move.l usp,a0
	movem.l a0/a2-a4/a6/d2-d7,-(sp)	;	save state
	move.l sp,U_DATA__U_SP(a5)	;	save pointer
	move.l U_DATA__U_PTAB(a5),-(sp)	;	parent p_tab

	; FIXME: for the single case with less swaps than processes the
	; out of swap case is one out (we must swap out to swap in so need
	; one swap free after the fork. Right now we deal with this in
	; Config by not setting it that way
	jsr swapout
	add.w #50,sp			;	throw the call frame and
					;	the frame we built for
					; 	switchin
	tst.w d0
	bne forked_up

	movel a5,-(sp)			;	U_DATA
	move.l 8(sp),a0
	move.l a0,-(sp)
	jsr makeproc
	addq #8,sp

	moveq #0,d0			;	child, ok
	clr.w runticks
	rts
forked_up:
	moveq #-1,d0			;	parent, failed
	rts

badswitchmsg: ascii "_switchin: FAIL"
            byte 13,10,0
.even
