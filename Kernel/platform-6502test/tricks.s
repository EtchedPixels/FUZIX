;
;	6502 version
;
        .export _switchout
        .export _switchin
        .export _dofork
	.export _ramtop


        .include "kernel.def"
        .include "../kernel02.def"

        .segment "COMMONMEM"

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; FIXME: make sure we optimise the switch to self case higher up the stack!
; 
; This function can have no arguments or auto variables.
_switchout:
	sei

        jsr _chksigs
;
;        ; save machine state
;        ldd #0 ; return code set here is ignored, but _switchin can 
;        ; return from either _switchout OR _dofork, so they must both write 
;        ; U_DATA__U_SP with the following on the stack:
;	pshs d
;	sts U_DATA__U_SP
;
;        ; set inint to false
;	lda #0
;	sta _inint
;
;        ; find another process to run (may select this one again) returns it
;        ; in X
;        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _trap_monitor

badswitchmsg: .asciiz "_switchin: FAIL\r\n"

; new process pointer is in X
_switchin:
;        orcc #0x10		; irq off
;
;	ldy P_TAB__P_PAGE_OFFSET+3,x
;	; FIXME: can we skip the usermaps here ?
;	stx 0xffa6		; map the process uarea we want
;	adda #1
;	stx 0xffa7
;	stx 0xffaf		; and include the kernel mapping
;
	; ------- No stack -------
        ; check u_data->u_ptab matches what we wanted
;	cmpx U_DATA__U_PTAB
;        bne switchinfail
;
	; wants optimising up a bit
;	lda #P_RUNNING
;	sta P_TAB__P_STATUS_OFFSET,x

;	lda #0
;	sta _runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
;        lds U_DATA__U_SP

;        puls x ; return code

        ; enable interrupts, if the ISR isn't already running
;	lda _inint
;        beq swtchdone ; in ISR, leave interrupts off
;	andcc #0xef
;swtchdone:
;        rts

switchinfail:
;	jsr outx
;        ldx #badswitchmsg
;        jsr outstring
;	; something went wrong and we didn't switch in what we asked for
;        jmp _trap_monitor

fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
;        orcc #0x10	 ; should already be the case ... belt and braces.

	; new process in X, get parent pid into y

;	stx fork_proc_ptr
;	ldy P_TAB__P_PID_OFFSET,x

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
;        pshs y ; y  has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
 ;       sts U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; --------- we switch stack copies in this call -----------
;	jsr fork_copy			; copy 0x000 to udata.u_top and the
					; uarea and return on the childs
					; common
	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
;	puls y

;        ldx fork_proc_ptr
;        jsr _newproc

	; any calls to map process will now map the childs memory

        ; runticks = 0;
;        clr _runticks
        ; in the child process, fork() returns zero.
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
;        rts

fork_copy:
;	ldd U_DATA__U_TOP
;	addd #0x0fff		; + 0x1000 (-1 for the rounding to follow)
;	lsra		
;	lsra
;	lsra
;	lsra
;	lsra			; bits 2/1 for 8K pages
;	anda #6			; lose bit 0
;	adda #2			; and round up to the next bank (but in 8K terms)
;
;	ldx fork_proc_ptr
;	ldy P_TAB__P_PAGE_OFFSET,x
;	; y now points to the child page pointers
;	ldx U_DATA__U_PAGE
;	; and x to the parent
;fork_next:
;	ld a,(hl)
;	out (0x11), a		; 0x4000 map the child
;	ld c, a
;	inc hl
;	ld a, (de)
;	out (0x12), a		; 0x8000 maps the parent
;	inc de
;	exx
;	ld hl, #0x8000		; copy the bank
;	ld de, #0x4000
;	ld bc, #0x4000		; we copy the whole bank, we could optimise
;				; further
;	ldir
;	exx
;	call map_kernel		; put the maps back so we can look in p_tab
; FIXME: can't map_kernel here - we've been playing with the maps, fix
; directly
;	suba #1
;	bne fork_next

;	ld a, c
;	out (0x13), a		; our last bank repeats up to common
	; --- we are now on the stack copy, parent stack is locked away ---
;	rts			; this stack is copied so safe to return on

	
