;
;	TMS9995 version for the Geneve
;
;	This is a lot less hairy than many platforms because we have some
;	stable memory at all times and lots of registers.
;
;	Sadly we dont have enough fast scratch to keep a workpad per process
;
;	TODO: all the fun stuff like swap and swap stacks
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../kernel-tms9995.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0xF000	; FIXME check

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; Our caller expects us to trash all the registers so we can party freely.
; We just need to be careful about the weird way X is our return and
; (S) is the caller return.
;
_plt_switchout:
	di
	stx	(-s)
	cla
	sta	(-s)			; Return code
	xfr	s,a
	sta	(_udata + U_DATA__U_SP)	; Save old SP

        ; find another process to run (may select this one again) returns it
        ; in A
        jsr	_getproc
	sta	(-s)		; stack it as an argument
        jsr	_switchin
        ; we should never get here
        jmp	_plt_monitor

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	process pointer is in 2(s) at entry.
;
_switchin:
	stx	(-s)

	jsr	___hard_di

	ldx	4(s)

	lda	P_TAB__P_PAGE_OFFSET(x)
	bnz	not_swapped

; TODO	jsr	_swapper

	lda	P_TAB__P_PAGE_OFFSET(x)

not_swapped:
        ; check u_data->u_ptab matches what we wanted


	; Load into the byte side
	ldab	P_RUNNING
	stab	P_TAB__P_STATUS_OFFSET(x)

	; fix up our pages as they may have changed
	lda 	P_TAB__P_PAGE_OFFSET(x)
	sta	(_udata + U_DATA__U_PAGE)
	lda 	P_TAB__P_PAGE_OFFSET+2(x)
	sta	(_udata + U_DATA__U_PAGE+2)

	cla
	sta	(_runticks)

	; Now do the big switch
	ldb	P_TAB__P_PAGE_OFFSET(x)
	ldbb	7(b)		; top page for 8K pages

	;	Stack and everything just flipped under us
	;	Don't touch the stack until we reload it below

	; TODO MMU op

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
	lda	(_udata + U_DATA__U_SP)
	xas

	ldb	(_udata + U_DATA__U_PTAB)
	sub	x,b
	bnz	switchinfail
	
	lda	(s+)
	ldx	(s+)

	ldbb	(_udata + U_DATA__U_ININTERRUPT)
        bnz	swtchdone	; in ISR, leave interrupts off
	jsr	___hard_ei
swtchdone:
        rsr

switchinfail:
	lda	badswitchmsg
        jsr	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_plt_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
	stx	(-s)
	ldx	4(s)

	; PID return
	lda	P_TAB__P_PID_OFFSET(x)
	sta	(-s)

	xfr	s,a
	sta	(_udata + U_DATA__U_SP)

	;
	;	Not smart about unused space. Wants fixing ?
	;

	;	udata is parent, proc data is child
	jsr	fork_copy

	stx	(-s)
	lda	_udata
	sta	(-s)
	jsr	_makeproc
	ldb	(s+)
	ldb	(s+)

        ; runticks = 0;
	cla
	sta	(_runticks)

        ; in the child process, fork() returns zero.
	ldb	(s+)
	ldx	(s+)
	cla
	rsr

fork_copy:
	; TODO
	rsr
