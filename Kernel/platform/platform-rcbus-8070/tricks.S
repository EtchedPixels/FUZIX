;
;	Using the 16K fixed model. Our setup is fairly normal except
;	that we've got stuff at FFC0-FF are internal unpaged RAM
;
;	TODO: save the user chunk of the dp RAM, also pass it to the
;	relocator in the lowlevel code so we can reloc it
;	(right now it's hard coded - need to use different reloctool etc)
;	Also need to copy it to the save on a fork.
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../../cpu-8070/kernel-8070.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word UDATA_STASH		; stash lives above ramtop

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_plt_switchout:
	and	s,=0xFE			; interrupts off
	; This processor is unusual in that it has an internal RAM
	; area at FFC0-FFFF we use for fast access to a few things
	; We reserve FFF0-FFFF to the user, but we need to save/restore
	; it on a switchin and fork
	ld	ea,:0xFFF0
	push	ea
	ld	ea,:0xFFF2
	push	ea
	ld	ea,:0xFFF4
	push	ea
	ld	ea,:0xFFF6
	push	ea
	ld	ea,:0xFFF8
	push	ea
	ld	ea,:0xFFFA
	push	ea
	ld	ea,:0xFFFC
	push	ea
	ld	ea,:0xFFFE
	push	ea
	ld	ea,=0
	push	ea			; return
	ld	ea,p1
	ld	p3,=_udata
	st	ea,U_DATA__U_SP,p3	; Save old SP

	jsr	map_proc_always
	jsr	stash_udata
	jsr	map_kernel

        ; find another process to run (may select this one again) returns it
        ; in d
        jsr	_getproc
	push	ea
        jsr	_switchin
        ; we should never get here
        jsr	_plt_monitor

stash_udata:
	ld	p2,=_udata
	ld	p3,=UDATA_STASH
	ld	a,=0
	st	a,:__tmp
stash_loop:
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	dld	a,:__tmp
	bnz	stash_loop
	ret

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry the stack holds the process to switch in
;
_switchin:
	and	s,=0xFE
	ld	ea,2,p1

	ld	p2,ea
	ld	p3,=_udata
	ld	ea,P_TAB__P_PAGE_OFFSET,p2
	sub	ea,U_DATA__U_PAGE,p3
	ld	t,ea
	or	a,e
	bz	nostash

	ld	ea,t
	or	a,e
	bnz	not_swapped

	push	p2
	jsr	_swapper
	pop	p2

not_swapped:
	jsr	map_proc_p2

	push	p2
	; We must do this inline as we will be changing stack under
	; ourselves and without stack

	ld	p2,=_udata
	ld	p3,=UDATA_STASH
	ld	a,=0
	st	a,:__tmp
unstash_loop:
	ld	ea,@2,p3
	st	ea,@2,p2
	ld	ea,@2,p3
	st	ea,@2,p2
	dld	a,:__tmp
	bnz	unstash_loop

	; Now get the stack back sane
	ld	p3,=_udata
	ld	ea,U_DATA__U_SP,p3
	ld	p1,ea

	jsr	map_kernel
nostash:
        ; check u_data->u_ptab matches what we wanted
	ld	ea,U_DATA__U_PTAB,p3
	sub	ea,0,p1		; it's top of stack right now
	or	a,e
	bnz	switchinfail

	pop	p2		; get the pointer back

	ld	a,=P_RUNNING
	st	a,P_TAB__P_STATUS_OFFSET,p2

	; fix up our pages as they may have changed
	ld	ea,P_TAB__P_PAGE_OFFSET,p2
	st	ea,U_DATA__U_PAGE,p3
	ld	ea,P_TAB__P_PAGE_OFFSET+2,p2
	st	ea,U_DATA__U_PAGE+2,p3

	ld	ea,=0
	ld	p2,=_runticks
	st	ea,0,p2

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
	ld	ea,U_DATA__U_SP,p3
	ld	p1,ea
	ld	a,U_DATA__U_ININTERRUPT,p3
        bz	swtchdone		; in ISR, leave interrupts off
	or	s,=1
swtchdone:
	pop	ea
	st	ea,:0xFFFE
	pop	ea
	st	ea,:0xFFFC
	pop	ea
	st	ea,:0xFFFA
	pop	ea
	st	ea,:0xFFF8
	pop	ea
	st	ea,:0xFFF6
	pop	ea
	st	ea,:0xFFF4
	pop	ea
	st	ea,:0xFFF2
	pop	ea
	st	ea,:0xFFF0
	pop	ea
        ret

switchinfail:
	ld	p2,=badswitchmsg
        jsr	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_plt_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
 	and s,=0xFE	 ; should already be the case ... belt and braces.


	ld	ea,2,p1
	ld	p2,ea
	ld	p3,=_udata

	ld	ea,P_TAB__P_PID_OFFSET,p2
	push	ea		; Push the PID

	; Build the saved direct page frame

	ld	ea,:0xFFF0
	push	ea
	ld	ea,:0xFFF2
	push	ea
	ld	ea,:0xFFF4
	push	ea
	ld	ea,:0xFFF6
	push	ea
	ld	ea,:0xFFF8
	push	ea
	ld	ea,:0xFFFA
	push	ea
	ld	ea,:0xFFFC
	push	ea
	ld	ea,:0xFFFE
	push	ea

	ld	ea,p1
	st	ea,U_DATA__U_SP,p3

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes p2 still holds the new process ptr
	;

	jsr	fork_copy

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.

	pop	ea		; pod the direct page fraeme
	pop	ea
	pop	ea
	pop	ea
	pop	ea
	pop	ea
	pop	ea
	pop	ea
	pop	ea		; and the pid

	push	p2		; arguments for C
	ld	ea,=_udata
	push	ea
        jsr	_makeproc
	pop	ea
	pop	ea

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	ld	ea,=0
	ld	p2,=_runticks
	st	ea,0,p2

        ; in the child process, fork() returns zero.

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        ret

;
;	On entry p2 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number. This wants optimising to avoid copying all the unused
;	space!
;
;	Copy memory
;
;	p2 is the process pointer
;
fork_copy:
	ld	a,P_TAB__P_PAGE_OFFSET,p2
	xch	e,a
	ld	p3,=_udata
	ld	a,U_DATA__U_PAGE,p3
	jsr	bank2bank
	ld	a,P_TAB__P_PAGE_OFFSET+1,p2
	xch	e,a
	ld	p3,=_udata
	ld	a,U_DATA__U_PAGE+1,p3
	jsr	bank2bank
	ld	a,P_TAB__P_PAGE_OFFSET+2,p2
	xch	e,a
	ld	p3,=_udata
	ld	a,U_DATA__U_PAGE+2,p3
	jsr	bank2bank

	jsr	map_proc_p2
	jsr	stash_udata
	jmp	map_kernel_di

bank2bank:
	push	p2
	ld	p3,=0xFE79
	st	ea,0,p3		; 0x4000 is now our source, 0x8000 dest
	ld	p2,=0x4000	; Start address
	ld	p3,=0x8000
copyloop:
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,@2,p2
	st	ea,@2,p3
	ld	ea,p3
	xch	a,e		; get top byte
	bp	copyloop	; sign +ve  (ie below 0x8000)
	pop	p2
	ret

