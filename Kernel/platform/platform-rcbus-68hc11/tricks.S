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
#include "../kernel-hc11.def"

	.section .common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
ramtop:
	.word 0xF000

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
plt_switchout:
	sei
	clra
	psha
	psha				; Return code
	sts udata + U_DATA__U_SP	; Save old SP

        ; find another process to run (may select this one again) returns it
        ; in d
        jsr getproc
        jsr switchin
        ; we should never get here
        jsr plt_monitor

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

	ldd	P_TAB__P_PAGE_OFFSET,x
	subd	#0		;compare with 0
	bne	not_swapped
	;
	;	Fun and games
	;	We allocate a 16K page to use as the common when we swap
	;	in. We then copy the common into it, switch into it and
	;	start executing on our copy of the common.
	;
	;	Notes:
	;	We have interrupts off so we can flip the bank before
	;	we update cur_map. This is needed or we update the wrong
	;	common space.
	;
	;	OPTIMIZE
	;	We do have internal unpaged RAM at F040-F1FF but we'll
	;	make use of that for truely common data once this all works.
	;
	pshx			; save proc ptr
	jsr	get_common	; reallocate dead page - returns new common
				; in B
	pulx			; get proc ptr back
	lds	#swapstack	; onto temporary valid stack
	stab	0xFE7B		; and switch onto new common
	stab    cur_map+3
	stx	switch_proc_ptr	;
	xgdx			; switch_proc_ptr back into D
	jsr	swap_finish
	ldx	switch_proc_ptr
	; fix up our pages as they may have changed. We don't need to
	; update their MMU mapping as we will flip maps before we need them
	ldd P_TAB__P_PAGE_OFFSET,x
	std udata + U_DATA__U_PAGE
	ldd P_TAB__P_PAGE_OFFSET+2,x
	std udata + U_DATA__U_PAGE+2
	;
	; At this point the new process is in memory and as we fixed up
	; the udata mappings, cur_map and switch_proc_ptr the behaviour from
	; this point onwards is the same
	;
not_swapped:
	; Get our bank
	ldab	P_TAB__P_PAGE_OFFSET+3,x	; top page
	; On this instruction we change the memory under us to the other
	; common (containing a copy of the same code) and a different stack
	; We cannot do this out of line as our stack is no longer valid
	;
	; Stuff like switch_proc_ptr also goes invalid here, so be careful.
	; We carry it across in X instead
	;
	stab	0xFE7B
	stab	cur_map+3
	; Now get the stack back sane
	lds	udata + U_DATA__U_SP

        ; check u_data->u_ptab matches what we wanted
	cpx	udata + U_DATA__U_PTAB
	bne	switchinfail

	ldab	#P_RUNNING
	stab	P_TAB__P_STATUS_OFFSET,x

	clra
	clrb
	std runticks

        ; restore machine state -- note we may be returning from either
        ; switchout or dofork
        lds udata + U_DATA__U_SP	; not needed ?
	ldaa udata + U_DATA__U_ININTERRUPT
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula			; recover return code
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

	std fork_proc_ptr
	xgdx

	ldx P_TAB__P_PID_OFFSET,x
	pshx				; Push the PID

	sts udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	jsr fork_copy

	; We put the same stack into the new common so when we switch all
	; will be good

	pulx

	ldx fork_proc_ptr
	ldab P_TAB__P_PAGE_OFFSET+3,x
	stab 0xFE7B	; switch common
	;
	; Move on to the new stack copy
	;
	stab cur_map+3
	xgdx		; into D as argument to makeproc
	ldx #udata	; push udata onto the new mapped stack
	pshx
        jsr makeproc
	pulx

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	clra
	clrb
	std runticks

        ; in the child process, fork() returns zero.

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	On entry ptr1 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number.
;
;	Copy memory
;
fork_copy:
	ldx fork_proc_ptr
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab udata+U_DATA__U_PAGE
	bsr bank2bank

	ldaa P_TAB__P_PAGE_OFFSET+1,x
	cmpa P_TAB__P_PAGE_OFFSET,x	; are we done ?
	beq same_page
	ldab udata+U_DATA__U_PAGE+1
	bsr bank2bank

	ldaa P_TAB__P_PAGE_OFFSET+2,x
	cmpa P_TAB__P_PAGE_OFFSET+1,x	; are we done ?
	beq same_page
	ldab udata+U_DATA__U_PAGE+2
	bsr bank2bank

	; OPTIMIZE: don't need to copy all of this block
	ldaa P_TAB__P_PAGE_OFFSET+3,x
	cmpa P_TAB__P_PAGE_OFFSET+2,x	; are we done ?
	beq same_page
	ldab udata+U_DATA__U_PAGE+3
	bsr bank2bank

same_page:
	jmp map_kernel_di

;
;	Copy 16K from page B to page A. Caller is responsible for putting
;	banks back sane. This is akin to the 6803 code except that we have
;	the y register we can use to slightly speed up the copying and avoid
;	patching the code
;
bank2bank:
	pshx			; save pointer
	ldx #0x4000		; Start address
copyloop:
	stab 0xFE79		; Now switch to source bank
	ldy ,x
	staa 0xFE79		; Switch bank to destination
	sty ,x
	inx
	inx
	cpx #0x8000		; End at 0x8000
	bne copyloop
	pulx
	ldab #0x21
	stab 0xFE79		; Put page back FIXME hardcoded is ugly
	rts
;
;	These need to live in common space as we have user mapped some
;	of the time we access them. Ideally we'd move these to a special
;	kernel DP area in common iram: TODO
;
switch_proc_ptr: .word 0

;
;	128 byte swap stack
;
	.ds 64
swapstack:
	.byte 0
