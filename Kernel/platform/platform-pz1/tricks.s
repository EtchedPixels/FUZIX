;
;	6502 version
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop
	.export _create_init_common

	.import _chksigs
	.import _plt_monitor

	.import map_kernel

	.import	_makeproc
	.import _getproc
	.import _runticks
	.import outstring
	.import outxa
	.import outcharhex

	.import _udata
	.import _use_mvn

	.import pushax
	.import incsp2

        .include "kernel.def"
        .include "../../cpu-6502/kernel02.def"
	.include "zeropage.inc"
	.include "ports.def"

        .segment "COMMONMEM"

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word $FE00

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
	sta	ptr1
	stx	ptr1+1
	; Take a second saved set as we are going to swap stacks and ZP
	; with a CPU that hasn't got sufficient registers to keep it on
	; CPU
	sta	switch_proc_ptr
	stx	switch_proc_ptr+1
;	jsr	outxa
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y
;	jsr	outcharhex
	sta	PORT_BANK_0		; switches zero page, stack memory area
	; ------- No valid stack, new ZP ----- stack must not be used

	; Set ptr1 back up (the old ptr1 was on the other ZP)
	lda	switch_proc_ptr
	sta	ptr1
	lda	switch_proc_ptr+1
	sta	ptr1+1

        ; check u_data->u_ptab matches what we wanted
	lda	_udata + U_DATA__U_PTAB
	cmp	ptr1
	bne	switchinfail
	lda	_udata + U_DATA__U_PTAB+1
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
        ldx _udata + U_DATA__U_SP
	txs

	; ------- Stack now valid again

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
	stx _udata + U_DATA__U_SP

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
	sta PORT_BANK_0			; switch to child and child stack
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

	jsr pushax

	lda #<_udata
	ldx #>_udata

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

;
;	On entry ptr1 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number. This wants optimising to avoid copying all the unused
;	space!
;
fork_copy:
	ldy #P_TAB__P_PAGE_OFFSET
	lda (ptr1),y		; child->p_pag[0]
	sta PORT_BANK_2		; 8000
	sta tmp1
	lda _udata + U_DATA__U_PAGE
	sta PORT_BANK_1		; 4000
	sta tmp2

	; Now use that window to copy 48K from 0000-BFFF

	jsr bank2bank		; copies 16K

	inc tmp1
	inc tmp2
	lda tmp1
	sta PORT_BANK_2
	lda tmp2
	sta PORT_BANK_1
	jsr bank2bank		; second 16K

	inc tmp1
	inc tmp2
	lda tmp1
	sta PORT_BANK_2
	lda tmp2
	sta PORT_BANK_1
	jsr bank2bank		; third 16K

	inc tmp1
	inc tmp2
	lda tmp1
	sta PORT_BANK_2
	lda tmp2
	sta PORT_BANK_1
	jsr bank2bank		; final block

	jmp map_kernel		; put the kernel mapping back as it should be
	
bank2bank:			; copy 16K from the block mapped
				; at 4000 to the block at 8000
				; Interrupts need to be off right now
				; For 6502 we can FIXME this as we fix fork
				; and interrupts in the core by fixing the
				; interrupt save/restore logic
				; For 65C816 we'd need a 16bit mode IRQ
				; wrapper which would be scary code
	lda _use_mvn
	beq bank2bank_6502


	.p816

	clc
	xce
	rep #$30
	; Stack is now insane so good job interrupts are off
	.a16
	.i16
	ldx #$4000
	ldy #$8000
	lda #$4000
	mvn #0,#0		; Bank is irrelevant as A16-A23 are unwired
	sep #$30
	.a8
	.i8
	sec
	xce
	rts

	.p02

bank2bank_6502:
	lda #$40
	sta ptr3+1
	lda #$80
	sta ptr4+1
	lda #0
	sta ptr3
	sta ptr4
	tay
	ldx #$40		;	64 x 256 bytes  = 16K
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

_create_init_common:
	lda #0
	sta PORT_BANK_1		;	set the map for 0x4000
	lda #4
	sta PORT_BANK_2		;	and 0x8000
	jsr bank2bank
	jmp map_kernel
;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch, and we need it to live about 0x4000
;
;	For now BSS will do but we ought to have a proper high space or
;	something.
;
	.bss

switch_proc_ptr: .word 0
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
