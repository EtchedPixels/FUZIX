;
;	6809 version
;
        .module tricks

	#imported
        .globl _swapout
        .globl _newproc
        .globl _chksigs
        .globl _getproc
        .globl _trap_monitor
        .globl _inint
        .globl map_kernel
        .globl map_process
        .globl map_process_a
        .globl map_process_always

	# exported
        .globl _switchout
        .globl _switchin
        .globl _dofork
	.globl _ramtop


        include "kernel.def"
        include "../kernel09.def"

	.area .common

_ramtop:
	.dw 0

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; FIXME: make sure we optimise the switch to self case higher up the stack!
; 
; This function can have no arguments or auto variables.
_switchout:
	orcc #0x10		; irq off
        jsr _chksigs

        ; save machine state
        ldd #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
	pshs d
	sts U_DATA__U_SP	; this is where the SP is restored in _switchin

        ; set inint to false
	lda #0
	sta _inint

	; Stash the uarea into process memory bank
	jsr map_process_always
	sty _swapstack+2

	ldx #U_DATA
	ldy #U_DATA_STASH
stash	ldd ,x++
	std ,y++
	cmpx #U_DATA+U_DATA__TOTALSIZE
	bne stash
	ldy _swapstack+2

	; get process table in
	jsr map_kernel

        ; find another process to run (may select this one again) returns it
        ; in X
        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _trap_monitor

_swapstack .dw 0
	.dw 0

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13
	    .db 10
	    .db 0

; new process pointer is in X
_switchin:
        orcc #0x10		; irq off

	;pshs x
	stx _swapstack
	; get process table - must be in already from switchout
	; jsr map_kernel
	lda P_TAB__P_PAGE_OFFSET+1,x		; LSB of 16-bit page no
	jsr map_process_a
	
	; fetch uarea from process memory
	sty _swapstack+2
	ldx #U_DATA_STASH
	ldy #U_DATA
stashb	ldd ,x++
	std ,y++
	cmpx #U_DATA_STASH+U_DATA__TOTALSIZE
	bne stashb
	ldy _swapstack+2

	; get back kernel page so that we see process table
	jsr map_kernel

	;puls x
	ldx _swapstack
        ; check u_data->u_ptab matches what we wanted
	cmpx U_DATA__U_PTAB
        bne switchinfail

	lda #P_RUNNING
	sta P_TAB__P_STATUS_OFFSET,x

	lda #0
	sta _runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds U_DATA__U_SP
        puls x ; return code

        ; enable interrupts, if the ISR isn't already running
	lda _inint
        beq swtchdone ; in ISR, leave interrupts off
	andcc #0xef
swtchdone:
        rts

switchinfail:
	jsr outx
        ldx #badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _trap_monitor

	.area .data
fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

	.area .common
;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
;	FIXME: preserve y
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        orcc #0x10	 ; should already be the case ... belt and braces.

	; new process in X, get parent pid into y

	stx fork_proc_ptr
	ldx P_TAB__P_PID_OFFSET,x

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        pshs x ; x  has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        sts U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	ldx U_DATA__U_PTAB		; parent
; FIXME
	; jsr _swapout			; swap the parent out, leaving
					; it in memory as the child

	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
	puls x

        ldx fork_proc_ptr
        jsr _newproc

	; any calls to map process will now map the childs memory

        ; runticks = 0;
        clr _runticks
        ; in the child process, fork() returns zero.
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

