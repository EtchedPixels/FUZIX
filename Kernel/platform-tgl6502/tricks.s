;
;	6502 version
;
        .export _switchout
        .export _switchin
        .export _dofork
	.export _ramtop

	.import _chksigs
	.import _trap_monitor

	.import map_kernel

	.import	_newproc
	.import _getproc
	.import _runticks
	.import _inint
	.import outstring
	.import outxa
	.import outcharhex

        .include "kernel.def"
        .include "../kernel02.def"
	.include "zeropage.inc"

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
	stx U_DATA__U_SP	; Save it

        ; set inint to false
	lda #0
	sta _inint

        ; find another process to run (may select this one again) returns it
        ; in x,a
        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _trap_monitor

badswitchmsg: .byte "_switchin: FAIL"
	.byte 13, 10, 0

;
;	On entry x,a holds the process to switch in
;
_switchin:
	sei
	sta	ptr1
	stx	ptr1+1
	; Take a second saved set as we are going to swap stacks and ZP
	; with a CPU that hasn't got sufficient registers to keep it on
	; CPU
	sta	switch_proc_ptr
	stx	switch_proc_ptr+1
	jsr	outxa
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y
;	pha
;	jsr	outcharhex
;	pla
	sta	$FF8A		; switches zero page, stack memory area
	; ------- New stack and ZP -------

	; Set ptr1 back up (the old ptr1 was on the other ZP)
	lda	switch_proc_ptr
	sta	ptr1
	lda	switch_proc_ptr+1
	sta	ptr1+1

        ; check u_data->u_ptab matches what we wanted
	lda	U_DATA__U_PTAB
	cmp	ptr1
	bne	switchinfail
	lda	U_DATA__U_PTAB+1
	cmp	ptr1+1
	bne	switchinfail

	lda	#P_RUNNING
	ldy	#P_TAB__P_STATUS_OFFSET
	sta	(ptr1),y

	lda #0
	sta _runticks
	sta _runticks+1

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ldx U_DATA__U_SP
	txs
	pla
	sta sp+1
	pla
	sta sp
	lda _inint
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
        jmp _trap_monitor

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
	stx U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes ptr1 still holds the new process ptr
	;

	jsr fork_copy

	; --------- we switch stack copies here -----------
        lda fork_proc_ptr
	ldx fork_proc_ptr+1
	sta ptr1
	stx ptr1+1
	ldy #P_TAB__P_PAGE_OFFSET
	lda (ptr1),y
	sta $FF8A			; switch to child and child stack
					; and zero page etc
	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
	pla
	pla
	pla
	pla

        lda fork_proc_ptr
	ldx fork_proc_ptr+1
;
;	FIXME: turn these into a C argument!
;
        jsr _newproc

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

;
;	On entry ptr1 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number.
;
fork_copy:
	ldx U_DATA__U_PAGE
	ldy #P_TAB__P_PAGE_OFFSET
	lda (ptr1),y		; child->p_page
	tay
	lda #0			; each bank is 56K
copy_loop:
	stx $FF8C		; 0x4000
	sty $FF8D		; 0x6000
	pha			; Oh for a 65C02 8)
	tya
	pha
	txa
	pha
	jsr bank2bank		; copies 8K
	pla
	tax
	pla
	tay
	pla
	inx
	iny
	clc
	adc #1
	cmp #7
	bne copy_loop
	jmp map_kernel		; put the kernel mapping back as it should be
	
bank2bank:			;	copy 4K between the blocks mapped
				;	at 0x4000 and 0x6000
	lda #$40
	sta ptr3+1
	lda #$60
	sta ptr4+1
	lda #0
	sta ptr3
	sta ptr4
	tay
	ldx #$20		;	32 x 256 bytes = 8K
copy1:
	lda (ptr3),y
	sta (ptr4),y
	iny
	bne copy1
	inc ptr3+1
	inc ptr4+1
	dex
	bne copy1
	rts

;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch
;
	.data

switch_proc_ptr: .word 0
