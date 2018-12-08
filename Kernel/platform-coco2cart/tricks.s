;
;	TODO: udata needs to be in common space so swap needs to be smarter
;	about split udata
;
        .module tricks

	# imported
        .globl _makeproc
        .globl _chksigs
        .globl _getproc
        .globl _platform_monitor
        .globl _inint
        .globl map_kernel
        .globl map_process
        .globl map_process_a
        .globl map_process_always
        .globl copybank
	.globl _nready
	.globl _inswap
	.globl _platform_idle
	.globl _udata

	# exported
        .globl _platform_switchout
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
_platform_switchout:
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
        jsr _platform_monitor

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

	lds #$0200		;	$1xx is vectors and swap stack

	ldx U_DATA__U_PTAB
	ldx P_TAB__P_PAGE_OFFSET+1,x
	beq not_swapout		;	it's dead don't swap it out
	ldx U_DATA__U_PTAB
	andcc #0xef
	jsr _swapout		;	swapout(pptr)
not_swapout:
	ldx newpp
	andcc #0xef
	jsr _swapper		; 	fetch our process
	ldx newpp
	lda #1
	sta P_TAB__P_PAGE_OFFSET+1,x	; marked paged in
not_swapped:
	orcc #0x10
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
        jmp _platform_monitor

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

	; new process in X, get parent pid into y

	stx fork_proc_ptr
	ldx P_TAB__P_PID_OFFSET,x

        ; Save the stack pointer and critical registers (Y and U used by C).
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        pshs x,y,u ;  x has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with X (ie return code) containing the child PID.
        ; Hurray.
        sts U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	ldx U_DATA__U_PTAB
	;
	; FIXME: review what is needed for IRQ safety here before we turn
	; on IRQs during the swapout
	;
	inc _inswap
	lds #$0200		;	Use the swap stack
	andcc #0xef
	jsr _swapout
	orcc #0x10
	dec _inswap
	lds U_DATA__U_SP

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
	cmpx #0
	bne forked_up

	puls x
	; We are now in the kernel child context


	ldx #_udata
	pshs x
        ldx fork_proc_ptr
        jsr _makeproc
	puls x

	; any calls to map process will now map the childs memory

        ; in the child process, fork() returns zero.
	ldx #0
        ; runticks = 0;
	stx _runticks
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
	puls y,u,pc
forked_up:
	puls x
	ldx #0
	puls y,u,pc

