;
;	65C816 version
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../../cpu-65c816/kernel816.def"

	.65c816
	.a16
	.i16

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word	0xC000	; for now

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_plt_switchout:
	sei

;
;	Put the C stack on the CPU stack, and store that in U_SP
;
	lda #0		; return code
	pha
	phy		; C stack
	tsx
	; TODO doesn't match core code - need to fix core
	stx	U_DATA__U_SP	; Save it

        ; find another process to run (may select this one again) returns it
        ; in A
        jsr	_getproc
        jsr	_switchin
        ; we should never get here
        jsr	_plt_monitor

badswitchmsg:
	.ascii	"_switchin: FAIL"
	.byte	13
	.byte	10
	.byte	0

;
;	On entry A holds the process to switch in
;
_switchin:
	sei
	; Take a second saved set as we are going to swap stacks and ZP
	; with a CPU that hasn't got sufficient registers to keep it on
	; CPU
	tax

	sep	#0x20
	.a8
	lda	P_TAB__P_PAGE_OFFSET+3,x
	sta	0xFE7B
	rep	#0x20
	.a16

	; ------- No valid stack, new DP ----- stack must not be used

	stx	switch_proc_ptr		; in new common space

        ; check u_data->u_ptab matches what we wanted
	lda	U_DATA__U_PTAB
	cmp	switch_proc_ptr
	bne	switchinfail

	sep	#0x20
	.a8
	lda	#P_RUNNING
	sta	P_TAB__P_STATUS_OFFSET,x
	rep	#0x20
	.a16

	stz	_runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork

        ldx	U_DATA__U_SP
	txs

	; ------- Stack now valid again
	ply		; C stack now valid, return code is TOS

	rep	#0x20
	.a8
	lda	U_DATA__U_ININTERRUPT
	sep	#0x20
	.a16
        beq	swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pla				; return code
        rts

switchinfail:
        ldx	#badswitchmsg
        jsr	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_plt_monitor


;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        sei		; ints off for a bit (need to improve this)

	ldx	0,y		; dofork(new_proc)
	stx	fork_proc_ptr	; child

	lda	P_TAB__P_PID_OFFSET,x
	pha			; save child pid for return

	iny			; remove argument
	iny
	phy			; save the C stack pointer

	tsx
	stx U_DATA__U_SP	; remember stack

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes ptr1 still holds the new process ptr
	;

	jsr fork_copy

	; --------- we switch stack copies here -----------
        ldx fork_proc_ptr
	sep #$20
	.a8
	lda P_TAB__P_PAGE_OFFSET + 3,x
	sta $FE7B			; switch to child/child stack/DP/S
	rep #$20
	.a16
	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
	pla
	pla

        lda fork_proc_ptr	; this was copied into the new map
	dey
	dey
	sta 0,y

        jsr _makeproc
	; increments y itself

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	stz _runticks

        ; in the child process, fork() returns zero.
	lda #0

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	Copy the process between banks
;	X = child we are creating
;	U_DATA holds parent
;
fork_copy:
	sep #0x20
	.a8
	lda U_DATA__U_PAGE
	sta $FE79		; source (4000)
	lda P_TAB__P_PAGE_OFFSET,x
	sta $FE7A		; destination (8000)
	jsr pagecopy
	lda U_DATA__U_PAGE+1
	sta $FE79
	lda P_TAB__P_PAGE_OFFSET+1,x
	sta $FE7A
	jsr pagecopy
	lda U_DATA__U_PAGE+2
	sta $FE79
	lda P_TAB__P_PAGE_OFFSET+2,x
	sta $FE7A
	jsr pagecopy
	lda U_DATA__U_PAGE+3
	sta $FE79
	lda P_TAB__P_PAGE_OFFSET+3,x
	sta $FE7A
	jsr pagecopy
	rep #0x20
	.a16
	jmp map_kernel		; put the kernel mapping back as it should be
	

pagecopy:			; copy 16K from the block mapped
				; at 4000 to the block at 8000
				; relies on interrupts being off
	rep #0x20
	.a16
	phx
	phy
	ldx #0x4000
	ldy #0x8000
	txa
	mvn #0,#0
	ply
	plx
	sep #0x20
	.a8
	rts

	.a16

; These switch with bank so care is needed in their use

switch_proc_ptr:
	.word 0
fork_proc_ptr:
	.word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
