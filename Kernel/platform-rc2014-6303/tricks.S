;
;	6303 version
;
        .export _platform_switchout
        .export _switchin
        .export _dofork
	.export _ramtop
	.export _create_init_common

#include "kernel.def"
#include "../kernel-6303.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0xF000

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_platform_switchout:
	sei
	sts _udata + U_DATA__U_SP	; Save it

        ; set inint to false
	clr _inint

        ; find another process to run (may select this one again) returns it
        ; in d
        jsr _getproc
	pshb
	psha
        jsr _switchin
        ; we should never get here
        jsr _platform_monitor

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry x,a holds the process to switch in
;
_switchin:
	sei
	tsx
	ldx	3,x
	stx	switch_proc_ptr

	ldab	P_TAB__P_PAGE_OFFSET+1,x
	stab	0xFE78		; switches 8000-BFFF FIXME:

        ; check u_data->u_ptab matches what we wanted
	ldx	_udata + U_DATA__U_PTAB
	cpx	switch_proc_ptr
	bne	switchinfail

	ldab	#P_RUNNING
	ldx	switch_proc_ptr
	stab	P_TAB__P_STATUS_OFFSET,x

	clr _runticks
	clr _runticks+1

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds _udata + U_DATA__U_SP
	ldaa _inint
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula
	pulb
        rts

switchinfail:
	ldx	#badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _platform_monitor

; Must not put this in ZP ?
;
; Move to commondata ??
;
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
        sei	 ; should already be the case ... belt and braces.

	tsx
	ldx 3,x
	; new process in X, get parent pid into y

	stx fork_proc_ptr

	ldx P_TAB__P_PID_OFFSET,x
	pshx

	; FIXME - save compiler temporaries

	sts _udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes ptr1 still holds the new process ptr
	;

	jsr fork_copy

	; --------- we switch stack copies here -----------
        ldx fork_proc_ptr
	ldab P_TAB__P_PAGE_OFFSET,x
	stab 0xFE78			; switch to child and child stack
					; and zero page etc FIXME
	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
	pulx
	pulx

	ldx fork_proc_ptr
	pshx
	ldx #_udata
	pshx
        jsr _makeproc
	pulx
	pulx

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	clr _runticks
	clr _runticks+1

        ; in the child process, fork() returns zero.
	ldd #0

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	On entry ptr1 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number. This wants optimising to avoid copying all the unused
;	space!
;
fork_copy:
	; TODO
	rts

_create_init_common:
	ldab #32
	stab 0xFE79		;	set the map for 0x4000
	ldab #36
	stab 0xFE7A		;	and 0x8000
	jsr bank2bank
	jmp map_kernel

bank2bank:
	; TODO
	rts
;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch
;
	.data

switch_proc_ptr: .word 0
