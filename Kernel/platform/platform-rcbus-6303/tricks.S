;
;	6303 version
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
#include "../kernel-6303.def"

	.setcpu 6303

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0xF000		;???? Check should be 0xBE00 ?? FIXME

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
	ldx #_udata
	ldd #$BE00		; Ick FIXME hardcoded
	std work
stash_loop:
	ldd ,x
	inx
	inx
	pshx
	ldx work
	std ,x
	inx
	inx
	stx work
	pulx
	cpx #_udata+512
	bne stash_loop
	rts

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry the stack holds the process to switch in
;
_switchin:
	sei
	tsx
	ldx	2,x
	stx	switch_proc_ptr

	ldd	P_TAB__P_PAGE_OFFSET,x
	cmpa	_udata+U_DATA__U_PAGE
	beq nostash

	subd	@zero		;compare with 0
	bne	not_swapped
	pshx
	jsr	_swapper
	pulx
	ldx	switch_proc_ptr
	ldab	P_TAB__P_PAGE_OFFSET+1,x

not_swapped:
	jsr	map_proc_x

	; We must do this inline as we will be changing stack under
	; ourselves and without stack
	ldx	#$BE00
	ldd	#_udata
	std	work
unstash_loop:
	ldd	,x
	inx
	inx
	stx	work2
	ldx	work
	std	,x
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
	ldd P_TAB__P_PAGE_OFFSET,x
	std _udata + U_DATA__U_PAGE
	ldaa P_TAB__P_PAGE_OFFSET+2,x
	staa _udata + U_DATA__U_PAGE+2

	clra
	clrb
	std _runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds _udata + U_DATA__U_SP
	ldaa _udata + U_DATA__U_ININTERRUPT
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula			; recover return code
	pulb
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
	; new process in X, get parent pid into y

	stx fork_proc_ptr

	ldx P_TAB__P_PID_OFFSET,x
	pshx				; Push the PID

	sts _udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes ptr1 still holds the new process ptr
	;

	jsr fork_copy

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
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
	clra
	clrb
	std _runticks

        ; in the child process, fork() returns zero.

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	On entry fork_proc_ptr points to the process table of the child, and
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
	jsr map_proc_x
	jsr stash_udata
	jmp map_kernel_di

;
;	The 680x isn't very good at block copying. We can do tricks with
;	the stack pointer but lets avoid that for now. We could really do
;	with an MMU with split read/write mappings so we could ldx,stx the
;	same address.
;
;	In fact for the 6803/6303 it's actually better to pretend to have
;	an MMU that works like that and keep switching the MMU
;
;	Alas we can't use EIM here because it doesn't do extended addresses
;	and our MMU is in I/O space ($FExx)
;
;	Copy 16K from page B to page A. Caller is responsible for putting
;	banks back sane
;
bank2bank:
	pshx
	staa patch1+1		; Patch the dest bank
	stab patch2+1		; Patch the dest bank
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
	pulx
	rts

;
;	These need to live in common space as we have user mapped some
;	of the time we access them. Ideally we'd move these to a special
;	kernel DP area in common iram: TODO
;
work:	.word	0
work2:	.word	0

;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch
;
	.data

switch_proc_ptr: .word 0
