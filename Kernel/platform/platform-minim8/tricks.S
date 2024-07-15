;
;	68HC11 version
;
;	We do not save the temporaries. The direct page is tied to each
;	map so the direct page working registers are also per process. Only
;	the interrupt path needs to save and restore them.
;
;	We don't save a lot of stuff. The compiler assumes D X and Y will
;	be changed by the called function anyway. The soft register state
;	lives in DP so is switched with the task switch and copied by fork
;
        .globl plt_switchout
        .globl switchin
        .globl dofork
	.globl ramtop

#include "kernel.def"
#include "../../cpu-68hc11/kernel-hc11.def"

	.section .common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
ramtop:
	.word	0xF000

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; TODO - use the 32K split to optimize chunks of copies. It's complicated
; by the fact the full 32K isn't visible in each half however.
;
plt_switchout:
	sei
	clra
	psha
	psha				; Return code
	sts	udata + U_DATA__U_SP	; Save old SP

	ldx	#0xDE00			; Stash
	ldy	#udata
	ldab	udata + U_DATA__U_PAGE + 1
	; Patch it as there's not enough registers to hand
	stab	pstash+1
stash_loop:
	ldaa	,y
pstash:
	ldab	#0
	stab	MLATCH
	staa	,x
	ldab	#0x10
	stab	MLATCH
	inx
	iny
	cpx	#0xE000
	bne	stash_loop
        ; find another process to run (may select this one again) returns it
        ; in d
        jsr	getproc
        jsr	switchin
        ; we should never get here
        jsr	plt_monitor

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry D holds the process to switch in. This is slightly more
;	exciting that it first may appear. We are in the context of the old
;	task, running on its stack. When we reload the memory bank we switch
;	to our stack. We must avoid damaging anything so be careful how
;	we sequence events
;
switchin:
	sei
	std	switch_proc_ptr
	xgdx

	;	Using the same page as before
	ldd	P_TAB__P_PAGE_OFFSET,x
	cmpd	udata+U_DATA__U_PAGE
	beq	nostash

	subd	#0		;compare with 0
	bne	not_swapped

	jsr	swapper
	ldx	switch_proc_ptr
not_swapped:
	; Get our bank
	ldab	P_TAB__P_PAGE_OFFSET+1,x	; bank

	; Get our udata back
	; Self modfy as we are a register short
	stab	udata_copy + 1
	ldx	#0xDE00
	ldy	#udata
udata_copy:
	ldaa	#0
	staa	MLATCH
	ldab	,x
	ldaa	#0x10
	staa	MLATCH
	stab	,y
	inx
	iny
	cpx	#0xE000
	bne	udata_copy

	; Now get the stack back sane
	lds	udata + U_DATA__U_SP
nostash:
	ldx	switch_proc_ptr
        ; check u_data->u_ptab matches what we wanted
	cpx	udata + U_DATA__U_PTAB
	bne	switchinfail

	; Copy bank as it may have changed if we swapped
	ldd	P_TAB__P_PAGE_OFFSET,x
	std	udata + U_DATA__U_PAGE

	ldab	#P_RUNNING
	stab	P_TAB__P_STATUS_OFFSET,x

	clra
	clrb
	std	runticks

        ; restore machine state -- note we may be returning from either
        ; switchout or dofork
        lds	udata + U_DATA__U_SP	; not needed ?
	ldaa	udata + U_DATA__U_ININTERRUPT
        beq	swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula		; recover return code
	pulb
        rts

switchinfail:
	ldx #badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp plt_monitor

; Must not put this in ZP ?
;
; Move to commondata ??
;
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
dofork:
;        ; always disconnect the vehicle battery before performing maintenance
        sei	 ; should already be the case ... belt and braces.

	; new process in D, get parent pid
	std	fork_proc_ptr
	xgdx

	ldx	P_TAB__P_PID_OFFSET,x
	pshx				; Push the PID

	sts	udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	jsr	fork_copy

	; We put the same stack into the new common so when we switch all
	; will be good

	pulx

	ldx	fork_proc_ptr

	xgdx		; into D as argument to makeproc
	ldx	#udata	; push udata onto the new mapped stack
	pshx
        jsr	makeproc
	pulx

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	clra
	clrb
	std	runticks

        ; in the child process, fork() returns zero.

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	We have udata giving the parent and fork_proc_ptr the
;	child.
;
fork_copy:
	ldx	fork_proc_ptr
	ldaa	P_TAB__P_PAGE_OFFSET + 1,x
	ldab	udata+U_DATA__U_PAGE + 1
	ldx	#0x0000
copy_loop:
	stab	MLATCH
	ldy	,x
	staa	MLATCH
	sty	,x
	inx
	inx
	cpx	#0xDE00
	bne	copy_loop

	ldaa	#0x10
	staa	MLATCH

	;	Now copy our live udata into parent stash
	stab	upatch+1

	ldy	#udata
fork_udata:
	ldab	,y
upatch:
	ldaa	#0
	staa	MLATCH
	stab	,x
	ldaa	#0x10
	staa	MLATCH
	inx
	iny
	cpx	#0xE000
	bne	fork_udata
	rts

;	Can be here as we always read it from kernel map
switch_proc_ptr: .word 0

;
;	128 byte swap stack - can live anywhere
;
	.ds 64
swapstack:
	.byte 0
