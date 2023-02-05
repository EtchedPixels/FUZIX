;
;	6502 version
;
    .export _plt_switchout
    .export _switchin
    .export _dofork
	.export _ramtop

	.import _chksigs
	.import _plt_monitor

	.import map_kernel
	.import _swapper
	.import _swapout

	.import	_makeproc
	.import _getproc
	.import _runticks
	.import outstring
	.import outxa
	.import outcharhex
	.import pushax
	.import _udata

    .include "kernel.def"
    .include "../kernel02.def"
	.include "zeropage.inc"

; FIXME: review but all but some of the fork logic looks safe to go in code
    .segment "COMMONMEM"

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word $C000

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
_plt_switchout:
	sei

;
;	Put the C stack on the CPU stack, and store that in U_SP
;
	lda #0		; return code
	pha
	pha
	lda sp		; C stack
	pha
	lda sp+1
	pha
	tsx
	stx _udata + U_DATA__U_SP	; Save it

        ; find another process to run (may select this one again) returns it
        ; in x,a
        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _plt_monitor

badswitchmsg: .byte "_switchin: FAIL"
	.byte 13, 10, 0

;
;	On entry x,a holds the process to switch in
;
_switchin:
	sei
	; Take a second saved set as we are going to swap stacks and ZP
	; with a CPU that hasn't got sufficient registers to keep it on
	; CPU
	sta	switch_proc_ptr
	stx	switch_proc_ptr+1
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y

	bne	not_swapped

	lda	_udata + U_DATA__U_PTAB
	ldx	_udata + U_DATA__U_PTAB+1
	;
	; FIXME - need the extra logic to swap out the kernel Z/S bits of
	; interest.
	;
;FIXME	jsr	_swapoutudz
	;
	jsr	_swapout

	; We have to worry about stacks here. We need to load in the
	; kernel C stack and CPU stack for the new process over the one
	; we are running upon. On most processors we just select another
	; stack but for the CPU stack on 6502 we can't do this.
	;
	; We do have the alternate ZP and stack but remember those hold
	; the user space ZP and stack. What we actually do is gross
	;

	lda	switch_proc_ptr
	ldx	switch_proc_ptr + 1

	; After swapout the ZP and S in memory are the old user ones, that
	; means they are effectively free memory

	sta	$C009			; Switch to Alt ZP

	;
	;	Set up a plausible ZP and S. S points somewhere in the
	;	right area. We don't care exactly so don't reload.
	;
	lda	#<swapstack_top
	sta	sp
	ldx	#>swapstack_top
	sta	sp+1
	;
	; We now have a constructed environment to run the
	; swap helper on the alt stack to load the real ZP and S
	;
;FIXME	jsr	_swapudz		; Swap in real Udata/Stack via
					; special platform routine

	;
	; We then run the swapper in the normal fashion and it will not
	; touch our kernel ZP and S so all is good. Note that we are still
	; on the swapstack for C. We'll only fix that up at the end.
	;
	sta	$C008			; Back to real C stack

	ldx	_udata + U_DATA__U_SP	; Use the memory below the
					; CPU stack we swapped in
	txs

	lda	switch_proc_ptr
	ldx	switch_proc_ptr + 1

	; Swap in the user process and FIXME teach swapper about 6502 S/Z
	; user pages.

	jsr	_swapper

	

not_swapped:
	; Make sure we swapped in the right process
	lda	_udata + U_DATA__U_PTAB
	cmp	switch_proc_ptr
	bne	switchinfail
	ldx	_udata + U_DATA__U_PTAB+1
	cpx	switch_proc_ptr+1
	bne	switchinfail
	; XA holds the process ptr as a sidde effect, now construct a
	; pointer
	sta	ptr1
	stx	ptr1+1
	; Set it to running
	ldy	#P_TAB__P_STATUS_OFFSET
	lda	#P_RUNNING
	sta	(ptr1),y
	; And copy the page offset into the udata
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	#0
	lda	(ptr1),y
	sta	_udata + U_DATA__U_PAGE
	; Fix up the stack pointer from the one we are hiding in
	ldx	_udata + U_DATA__U_SP
	txs
	
	lda #0
	sta _runticks
	sta _runticks+1

	pla
	sta sp+1
	pla
	sta sp
	lda _udata + U_DATA__U_ININTERRUPT
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pla		; Return code
	tax
	pla
        rts

switchinfail:
	lda	ptr1+1
	jsr	outcharhex
	lda	ptr1
	jsr	outcharhex
        lda	#<badswitchmsg
	ldx	#>badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _plt_monitor


;
;	Do the hard work for a fork. Actually in a swap only environment
;	there isn't that much to do. We swap out the existing image as is
;	and it's the parent, and we keep the memory copy as child.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
        sei	 ; should already be the case ... belt and braces.

	; new process in X, get parent pid into y

	sta fork_proc_ptr
	stx fork_proc_ptr+1

	ldy #P_TAB__P_PID_OFFSET
	sta ptr1
	stx ptr1+1

        ; Save the stack pointer and critical registers.
	; 6502 at least doesn't have too many of those 8)

        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
	lda (ptr1),y
	pha
	iny
	lda (ptr1),y
	pha

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) the child PID.
	lda sp
	pha
	lda sp+1
	pha
	tsx
	stx _udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process so write it out to disk.

	lda fork_proc_ptr
	ldx fork_proc_ptr+1

	jsr _swapout

        ; now the save operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.

	pla
	pla
	pla
	pla

        lda fork_proc_ptr
	ldx fork_proc_ptr+1

	jsr pushax

	lda #<_udata
	ldx #>_udata

	; Perform the necessary magic to turn into the child
        jsr _makeproc

	; any calls to map process will now map the childs memory
        ; runticks = 0;

	lda #0
	sta _runticks
	sta _runticks+1

        ; in the child process, fork() returns zero.
	tax

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

	.data

switch_proc_ptr: .word 0
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
swapstack:
	.res 128,0
swapstack_top:
