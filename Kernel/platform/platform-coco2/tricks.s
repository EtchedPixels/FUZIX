;
;	TODO: udata needs to be in common space so swap needs to be smarter
;	about split udata
;
        .module tricks

	# imported
        .globl _makeproc
        .globl _chksigs
        .globl _getproc
        .globl _plt_monitor
        .globl _inint
        .globl map_kernel
        .globl map_process
        .globl map_process_a
        .globl map_process_always
        .globl copybank
	.globl _nready
	.globl _inswap
	.globl _plt_idle
	.globl _udata

	# exported
        .globl _plt_switchout
        .globl _switchin
        .globl _dofork
	.globl _ramtop

        include "kernel.def"
        include "../kernel09.def"

	.area .common

	; ramtop must be in common although not used here
_ramtop:
	.dw 0

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; 
; This function can have no arguments or auto variables.
_plt_switchout:
	orcc #0x10		; irq off

        ; save machine state, including Y and U used by our C code
        ldd #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
	pshs d,y,u
	sts U_DATA__U_SP	; this is where the SP is restored in _switchin

        ; find another (or same) process to run, returned in X
        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _plt_monitor

badswitchmsg:
	.ascii "_switchin: FAIL"
        .db 13
	.db 10
	.db 0

newpp   .dw 0

; new process pointer is in X
_switchin:
        orcc #0x10		; irq off

	stx newpp
	inc _inswap
	; get process table
	lda P_TAB__P_PAGE_OFFSET+1,x		; LSB of 16-bit page no

	cmpa #0
	bne not_swapped

	; Steal the interrupt stack. This means we have to keep interrupts
	; off whilst swapping but it's not clear we've got enough other
	; room!
	lds #$0100

	ldx U_DATA__U_PTAB
	ldx P_TAB__P_PAGE_OFFSET+1,x
	beq not_swapout		;	it's dead don't swap it out
	ldx U_DATA__U_PTAB
;;	andcc #0xef
	jsr _swapout		;	swapout(pptr)
not_swapout:
	ldx newpp
;;	andcc #0xef
	jsr _swapper		; 	fetch our process
	ldx newpp
	lda #1
	sta P_TAB__P_PAGE_OFFSET+1,x	; marked paged in
not_swapped:
;;	orcc #0x10
	dec _inswap
	; we have now new stacks so get new stack pointer before any jsr
	lds U_DATA__U_SP

	; get back kernel page so that we see process table
	jsr map_kernel

	ldx newpp
        ; check u_data->u_ptab matches what we wanted
	cmpx U_DATA__U_PTAB
        bne switchinfail

	lda #P_RUNNING
	sta P_TAB__P_STATUS_OFFSET,x

	; fix any moved page pointers
	lda P_TAB__P_PAGE_OFFSET+1,x
	sta U_DATA__U_PAGE+1

	ldx #0
	stx _runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds U_DATA__U_SP
        puls x,y,u ; return code and saved U and Y

        ; enable interrupts, if the ISR isn't already running
	lda U_DATA__U_ININTERRUPT
        bne swtchdone ; in ISR, leave interrupts off
	andcc #0xef
swtchdone:
        rts

switchinfail:
	jsr outx
        ldx #badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _plt_monitor

	.area .data

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

	.area .common
;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        orcc #0x10	 ; should already be the case ... belt and braces.

	; new process in X so make sure x is 0 in the child image

	stx fork_proc_ptr
	ldx #0		; child returns 0

        ; Save the stack pointer and critical registers (Y and U used by C).
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        pshs x,y,u ;  x has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in for the childt we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with X (ie return code) containing the child PID.
        ; Hurray.
        sts U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; Copy the parent properties into a temporary udata buffer
	jsr _tmpbuf
	tfr x,y
	ldu #_udata
udsave:
	ldd ,u++
	std ,x++
	cmpu #_udata+U_DATA__TOTALSIZE
	bne udsave

	; Buffer is in Y - make a new process using the buffer in Y
	; not the live updata
	pshs y
	ldx fork_proc_ptr
	jsr _makeproc

	; Now set up for the swapout
	pshs y		; callers can change args in C
	ldx fork_proc_ptr
	jsr _swapout_new

	leas 4,s	; clean up from both calls

	tfr y,x
	jsr _tmpfree

	; The child is now on disk, the parent is still running. This is
	; good, it reduces thrash
	ldx fork_proc_ptr
	ldd P_TAB__P_PID_OFFSET,x
	; Clean up and return
	puls x,y,u
	tfr d,x
	rts
