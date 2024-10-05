;
;	6809 version: TODO switch to 4 x 16K banking properly
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
        .globl map_process_always
	.globl _nready
	.globl _plt_idle
	.globl _udata
	.globl _get_common
	.globl _swap_finish

	# exported
        .globl _plt_switchout
        .globl _switchin
        .globl _dofork
	.globl _ramtop

        include "kernel.def"
        include "../../cpu-6809/kernel09.def"

	.area .commondata

	; ramtop must be in common although not used here
_ramtop:
	.dw 0

newpp   .dw 0

	.area .common

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
_plt_switchout:
	orcc	#0x10		; irq off

        ; save machine state, including Y and U used by our C code
	clra
	clrb			; return code set here is ignored, but _switchin can
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
	pshs	d,y,u
	sts	U_DATA__U_SP	; this is where the SP is restored in _switchin

        ; find another (or same) process to run, returned in X
        jsr	_getproc
        jsr	_switchin
        ; we should never get here
        jsr	_plt_monitor

badswitchmsg:
	.ascii "_switchin: FAIL"
	.db	13
	.db	10
	.db	0

; new process pointer is in X
_switchin:
        orcc	#0x10		; irq off

	stx	newpp
	; get process table
	ldd	P_TAB__P_PAGE_OFFSET,x		; Will be 0 in swap cases
	bne	not_swapped

	jsr	_get_common	; reallocate dead page for new common
				; B (FIXME check) is the new comon page
	ldx	newpp
	lds	#swapstack
	stb	0xFE7B		; top 16K switched to new bank
	stb	cur_map+3	; remember the new mapping
	stx	newpp		; put newpp back in the new bank

	jsr	_swap_finish	; void swap_finish(ptptr p)
	ldx	newpp
	lda	P_TAB__P_PAGE_OFFSET+1,x
	;	Fix up our pages as they may have changed whilst swapped
	ldd	P_TAB__P_PAGE_OFFSET,x
	std	U_DATA__U_PAGE
	ldd	P_TAB__P_PAGE_OFFSET+2,x
	std	U_DATA__U_PAGE+2

	; We are in memory and our mappings are fixed up. We can now rejoin
	; the normal flow but remember we are still on the swap stack

not_swapped:
	lda	P_TAB__P_PAGE_OFFSET+3,x
	sta	0xFE7B		; top 16K is now our memory

	; Set the correct stack pointer before any jsr
	lds	U_DATA__U_SP

        ; check u_data->u_ptab matches what we wanted
	cmpx	U_DATA__U_PTAB
        bne	switchinfail

	lda	#P_RUNNING
	sta	P_TAB__P_STATUS_OFFSET,x

	ldx	#0
	stx	_runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds	U_DATA__U_SP
        puls	x,y,u ; return code and saved U and Y

        ; enable interrupts, if the ISR isn't already running
	lda	U_DATA__U_ININTERRUPT
        bne	swtchdone ; in ISR, leave interrupts off
	andcc	#0xef
swtchdone:
        rts

switchinfail:
	jsr	outx
        ldx	#badswitchmsg
        jsr	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_plt_monitor

	.area	.data

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

	.area	.common
;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        orcc #0x10	 ; should already be the case ... belt and braces.

	; new process in X, get parent pid into y

	stx	fork_proc_ptr
	ldx	P_TAB__P_PID_OFFSET,x

        ; Save the stack pointer and critical registers (Y and U used by C).
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        pshs	x,y,u ;  x has p->p_pid from above, the return value in the parent

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with X (ie return code) containing the child PID.
        ; Hurray.
        sts	U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	jsr	fork_copy		; copy process memory to new bank
					; and save parents uarea

	; On return from the fork copy X points to the byte after the top
	; bank code

	lda	-1,x			; upper bank
	sta	$FE7B			; set to child common

	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
	puls	x

	ldx	#_udata
	pshs	x
        ldx	fork_proc_ptr
        jsr	_makeproc
	puls	x

	; any calls to map process will now map the childs memory

        ; in the child process, fork() returns zero.
	ldx	#0
        ; runticks = 0;
	stx	_runticks
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
	puls	y,u,pc

fork_copy:
; copy the process memory to the new bank and stash parent uarea to old bank
	ldx	fork_proc_ptr
	leax	P_TAB__P_PAGE_OFFSET,x	; pointer to pages
	ldu	#U_DATA__U_PAGE
	jsr	copybank		; copy low 16K
	jsr	copybank		; copy mid 16K
	jsr	copybank		; copy upper 16K
	jsr	copybank		; copy top 16K
	jmp	map_kernel		; fix up any map mess

;
;	TODO 6309 version ?
;
copybank:
	jsr	map_kernel		; process map is in kernel space
	lda	,u+
	ldb	,x+
	sta	0xFE79
	stb	0xFE7A
	pshs	x,u
	ldx	#0x4000
	ldu	#0x8000
copy:					; Performance matters here
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	cmpx	#$8000
	bne	copy
	puls	x,u,pc

	.ds	128
swapstack:
