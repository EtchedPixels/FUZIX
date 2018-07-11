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

	;
	;	Now switch task context
	;
	move.l P_TAB__P_UDATA_OFFSET(a0),a5
	move.l a5,udata_shadow

	;	Swap support needs to go here once we have the software
	;	MMU stable ?

	move.l P_TAB__P_UDATA_OFFSET(a0),a5
	move.l a0,-(sp)
	move.l a0,-(sp)
	jsr pagemap_switch
	addq #4,sp
	move.l (sp)+,a0

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
	; We are running as the parent, A5 = u_data pointer of parent.
	;
dofork:
	;
	; On a flat_mem system the software MMU already cloned the memory
	; map except that the child is not currently identity mapped. The
	; parent will return via its copy of the udata, but the child udata
	; is not mapped at the same address so we must build the child a
	; fake task switch frame that returns to user space.
	;
	move.l 4(sp),a0			;	child p_tab

	move.l P_TAB__P_UDATA_OFFSET(a0),a1	;	child udata

	;
	; Copy the parent udata into the child. Only copy the struct data.
	; Probably not worth saving registers and using movem to speed up.
	;

	movem.l a1/a5,-(sp)
	moveq #udata_copysize-1,d0
cplp:
	move.l (a5)+,(a1)+
	dbra d0,cplp

	movem.l (sp)+,a1/a5


	lea.l 512(a1),a0		;	top of child kernel stack

	;
	; Stack a task switch frame into a1
	;

	move.l 508(a5),-(a0)		;	trap frame
	move.l 504(a5),-(a0)		;	 ""    ""
	move.l 500(a5),-(a0)		;	A5
	move.l #child_rte,-(a0)		;	under the 3 copied words
	movem.l a0/a2-a4/a6/d2-d7,-(a0)	;	the faked switchin frame
	move.l a0,U_DATA__U_SP(a1)	;	set child frame pointer

	;
	; Now finish up the process creation
	;
	move.l	a5,-(sp)		;	save true udata
	move.l P_TAB__P_UDATA_OFFSET(a1),a5	;	child
	move.l a5,udata_shadow		; 	paranoia
	
	jsr newproc			;	Called with the child udata

	move.l U_DATA__U_PTAB(a5),a1	;	reload child pptr
	move.w P_TAB__P_PID_OFFSET(a1),d0	;	pid of child
	ext.l d0
	move.l (sp)+,a5			;	recover parent udata
	move.l a5,udata_shadow		; 	paranoia
	rts				; 	return as parent

;
;	This is called with the faked stack frame from dofork. All we have
;	on the stack is the A5 to restore and the fake stack frame. All the
;	other register restores will be done via the fork() syscall code in
;	userspace. The child returns this way not via syscall paths.
;
child_rte:
	move.l (sp)+,a5
	moveq #0,d0
	move.l d0,d1
	move.l d1,d2
	move.l d2,d3
	move.l d3,d4
	move.l d4,d5
	move.l d5,d6
	move.l d6,d7
	move.l d7,a0
	move.l a0,a1
	move.l a1,a2
	move.l a2,a3
	move.l a3,a4
	move.l a4,a6
	rte


badswitchmsg: ascii "_switchin: FAIL"
            byte 13,10,0
.even

udata_copysize equ U_DATA__COPYSIZE
