;
;	68HC11 fully banked (so no real 'common')
;
        .file "tricks"
	.mode mshort

        .globl newproc
        .globl chksigs
        .globl getproc
        .globl trap_monitor
        .globl inint
        .globl switchout
        .globl switchin
        .globl dofork
	.globl ramtop

        include "kernel.def"
        include "../kernel-hc11.def"

        .sect .data

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here

ramtop:
	.word 0

	.sect .code

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; FIXME: make sure we optimise the switch to self case higher up the stack!
; 
; This function can have no arguments or auto variables.
switchout:
	sei
        jsr chksigs

        ; save machine state
        ldx #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
	pshx
	sts U_DATA__U_SP

        ; set inint to false
	clr inint

        ; find another process to run (may select this one again) returns it
        ; in X
        jsr getproc
        jsr switchin
        ; we should never get here
        jsr trap_monitor

badswitchmsg: .ascii "switchin: FAIL"
            .byte 13
	    .byte 10
	    .byte 0

; new process pointer is in D
switchin:
	sei
	
	xgdx

	;
	; FIXME: implement dragon-nx uarea copy logic for HC11 via
	; eeprom copy helpers
	;

	; ------- No stack -------
        ; check u_data->u_ptab matches what we wanted
	cmpx U_DATA__U_PTAB
        bne switchinfail

	; wants optimising up a bit
	ldaa #P_RUNNING
	staa P_TAB__P_STATUS_OFFSET,x

	clr runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds U_DATA__U_SP


        ; enable interrupts, if the ISR isn't already running
	ldaa inint
        beq swtchdone ; in ISR, leave interrupts off
	cli
swtchdone:
        pulx ; return code
	xgdx
        rts

switchinfail:
	jsr outx
        ldx #badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp trap_monitor

fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
dofork:
	sei
	; new process is in D, get parent pid into d and process into x

	xgdx

	stx fork_proc_ptr
	ldd P_TAB__P_PID_OFFSET,x

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
	xgdx
        pshx	 ; x has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        sts U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; --------- we switch stack copies in this call -----------
	; d is the process pointer
	jsr fork_copy

	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
	pulx

        ldd fork_proc_ptr
        jsr newproc

	; any calls to map process will now map the childs memory

        ; runticks = 0;
        clr runticks
        ; in the child process, fork() returns zero.
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

fork_copy:
	; FIXME
	rts			; this stack is copied so safe to return on

	
