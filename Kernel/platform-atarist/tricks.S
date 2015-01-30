#include "../kernel-68000.def"


.globl switchout,switchin,dofork

.mri 1

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
; 
; This function can have no arguments or auto variables.
switchout:
        or #$0700,sr
        bsr chksigs
        ; save machine state

        clr.w -(sp) ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
	movem.l a0-a4/a6/d0-d7,-(sp)	; FIXME: trim to callee saves
	move.l sp,U_DATA__U_SP(a5)	; this is where the SP is restored in _switchin

        ; set inint to false
        clr.b inint

        ; find another process to run (may select this one again)
        bsr getproc

       	move.l a0,-(sp)		; CHECK compiler call/return rules
        bsr switchin

        ; we should never get here
        jmp trap_monitor

badswitchmsg: ascii "_switchin: FAIL"
            byte 13,10,0
swapped: ascii "_switchin: SWAPPED"
            byte 13,10,0

switchin:
        or #$0700,sr
	move.l 4(sp),a0		; task to switch to
	move.l P_TAB__P_UDATA_OFFSET(a0),a5

        ; check u_data->u_ptab matches what we wanted
	move.l 4(sp),d0
	cmp.l U_DATA__U_PTAB(a5),d0
	bne switchinfail

	; wants optimising up a bit
	move.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)

        ; runticks = 0
	clr.w runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        move.l U_DATA__U_SP(a5),sp
	movem.l (sp)+,a0-a4/a6/d0-d7	; FIXME: trim to callee saves here too
	add #2,sp			; drop return code dummy

        tst.b inint
        beq keepoff ; in ISR, leave interrupts off
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
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
;	We don't support fork() proper only vfork()
;
dofork:
        ; always disconnect the vehicle battery before performing maintenance
        or #$0700,sr ; should already be the case ... belt and braces.

	move.l 4(sp),a0
	move.l a0,fork_proc_ptr

        ; prepare return value in parent process -- HL = p->p_pid;
	move.w P_TAB__P_PID_OFFSET(a0),d0
	move.w d0,-(sp)
	movem.l a0-a6/d0-d7,-(sp)		; FIXME - callees !

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.

        move.l sp,U_DATA__U_SP(a5)
	move.l a5,P_TAB__P_UDATA_OFFSET(a0)

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; FIXME: allocation heap walk/copy for chains

	bsr bankfork			;	do the bank to bank copy

	; Returns with a5 set for the new process

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.

	add.l #58,sp			; FIXME: adjut when adjust movem's

        ; Make a new process table entry, etc.
        move.l fork_proc_ptr,-(sp)
        jsr newproc
        add.l #4,sp

        clr.w runticks
        ; in the child process, fork() returns zero.
	clr.w d0
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	This is related so we will keep it here. Copy the process memory
;	for a fork. Walk the heap allocations for this process and duplicate
;	them. We need to allow this to fail and ripple the error back nicely
;
bankfork:
	rts

fork_proc_ptr: long 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
