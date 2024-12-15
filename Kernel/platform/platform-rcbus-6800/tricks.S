;
;	6800 version
;
;	We do not save the temporaries. The direct page is tied to each
;	map so the direct page working registers are also per process. Only
;	the interrupt path needs to save and restore them.
;
;	The only ugly we have is that because we are using the 16Kx4 memory
;	banking model and our code density isn't quite good enough we need
;	to switch udata objects on task switches. With the 56K/8K MMU we
;	could avoid this!
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../../cpu-6800/kernel-6800.def"

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
_plt_switchout:
	sei
	clra
	psha
	psha				; Return code
	sts _udata + U_DATA__U_SP	; Save old SP

	jsr map_proc_always
	jsr stash_udata
	jsr map_kernel

        ; find another process to run (may select this one again) returns it
        ; in d
        jsr _getproc
	pshb
	psha
        jsr _switchin
        ; we should never get here
        jsr _plt_monitor

stash_udata:
	ldx #$BE00		; Ick FIXME hardcoded
	stx work
	ldx #_udata
stash_loop:
	ldaa ,x
	ldab 1,x
	inx
	inx
	stx work2
	ldx work
	staa ,x
	stab 1,x
	inx
	inx
	stx work
	ldx work2
	cpx #_udata+512
	bne stash_loop
	rts

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry the process number is on stack
;
_switchin:
	sei
	tsx
	ldx	2,x
	stx	switch_proc_ptr

	ldaa	P_TAB__P_PAGE_OFFSET,x
	ldab	P_TAB__P_PAGE_OFFSET+1,x
	cmpa	_udata+U_DATA__U_PAGE
	beq nostash

	tstb			;compare with 0
	bne	not_swapped
	tsta
	bne	not_swapped
	jsr	_swapper
	ldx	switch_proc_ptr
	ldab	P_TAB__P_PAGE_OFFSET+1,x

not_swapped:
	jsr	map_proc_x

	; We must do this inline as we will be changing stack under
	; ourselves and without stack
	ldx	#_udata
	stx	work
	ldx	#$BE00
unstash_loop:
	ldaa	,x
	ldab	1,x
	inx
	inx
	stx	work2
	ldx	work
	staa	,x
	stab	1,x
	inx
	inx
	stx	work
	ldx	work2
	cpx	#$C000		; BE000 + 0200
	bne	unstash_loop

	; Now get the stack back sane
	lds	_udata + U_DATA__U_SP
	jsr	map_kernel
nostash:
        ; check u_data->u_ptab matches what we wanted
	ldx	_udata + U_DATA__U_PTAB
	cpx	switch_proc_ptr
	bne	switchinfail

	ldab	#P_RUNNING
	ldx	switch_proc_ptr
	stab	P_TAB__P_STATUS_OFFSET,x

	; fix up our pages as they may have changed
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab P_TAB__P_PAGE_OFFSET+1,x
	staa _udata + U_DATA__U_PAGE
	stab _udata + U_DATA__U_PAGE+1
	ldaa P_TAB__P_PAGE_OFFSET+2,x
	staa _udata + U_DATA__U_PAGE+2

	clr _runticks
	clr _runticks+1

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds _udata + U_DATA__U_SP
	ldaa _udata + U_DATA__U_ININTERRUPT
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula			; recover return code
	pulb
	; We think we were called by switchout(void) so no arg cleanup
	; however this requires joy in the fork path where there is an
	; argument
        rts

switchinfail:
	ldx	#badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _plt_monitor

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
	ldx 2,x
	; new process in X, get parent pid into D

	stx fork_proc_ptr

	tsx
	ldab 1,x
	ldaa 0,x	; return address
	stab 3,x	; copy over the argument to remove it
	staa 2,x

	ldx fork_proc_ptr
	ldaa P_TAB__P_PID_OFFSET,x
	ldab P_TAB__P_PID_OFFSET+1,x

	tsx
	staa 0,x	; overwrite the space under the moved return
	stab 1,x	; address with the return value

	sts _udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes fork_proc_ptr still holds the new process ptr
	;

	jsr map_proc_always	; map parent
	jsr stash_udata		; save parent udata
	jsr map_kernel
	jsr fork_copy

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
	ins
	ins

	ldaa #>_udata
	ldab #<_udata
	pshb
	psha
	ldaa fork_proc_ptr
	ldab fork_proc_ptr+1
	pshb
	psha
        jsr _makeproc

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	clra
	clrb
	clr _runticks
	clr _runticks+1

        ; in the child process, fork() returns zero.

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
;	Copy memory
;
fork_copy:
	ldx fork_proc_ptr
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab _udata+U_DATA__U_PAGE
	bsr bank2bank
	inx
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab _udata+U_DATA__U_PAGE+1
	bsr bank2bank
	inx
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab _udata+U_DATA__U_PAGE+2
	bsr bank2bank

	ldx _udata+U_DATA__U_PTAB
	jmp map_kernel_di

;
;	The 6800 isn't very good at block copying. We can do tricks with
;	the stack pointer but lets avoid that for now. We could really do
;	with an MMU with split read/write mappings so we could ldx,stx the
;	same address.
;
;	Copy 16K from page B to page A. Caller is responsible for putting
;	banks back sane
;
;	One option if we want to take the irq latency is to do blocks
;	using S as a pointer.
;
bank2bank:
	stx work
	staa patch1+1		; Patch destination bank in
	stab patch2+1		; Patch source bank switch
	stab $FE79		; Switch to source bank
	ldx #$4000		; Start address
copyloop:
	ldab ,x
patch1:
	ldaa #0
	staa $FE79		; Switch bank to secondary
	stab ,x
	inx
patch2:
	ldaa #0
	staa $FE79		; And back
	cpx #$8000		; End at 0x8000
	bne copyloop
	ldx work
	rts

;
;	These need to live in common space as we have user mapped some
;	of the time we access them.
;
work:	.word	0
work2:	.word	0

;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch
;
	.data

switch_proc_ptr: .word 0
