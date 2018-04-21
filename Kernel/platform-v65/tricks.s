;
;	6502 version
;
        .export _platform_switchout
        .export _switchin
        .export _dofork
	.export _ramtop
	.export _create_init_common

	.import _chksigs
	.import _platform_monitor

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
	.word $E000

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_platform_switchout:
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
	stx U_DATA__U_SP	; Save it

        ; set inint to false
	lda #0
	sta _inint

        ; find another process to run (may select this one again) returns it
        ; in x,a
        jsr _getproc
        jsr _switchin
        ; we should never get here
        jsr _platform_monitor

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
;	pha
;	jsr	outcharhex
;	pla
	sta	$FE00		; switches zero page, stack memory area
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
	lda	#'*'
	sta	$FE20
	lda _inint
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pla		; Return code
	tax
	pla
        rts

switchinfail:
	lda	#'!'
	sta	$FE20
	lda	ptr1+1
	jsr	outcharhex
	lda	ptr1
	jsr	outcharhex
        lda	#<badswitchmsg
	ldx	#>badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp _platform_monitor

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
	sta $FE00			; switch to child and child stack
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
;	number. This wants optimising to avoid copying all the unused
;	space!
;
fork_copy:
	ldy #P_TAB__P_PAGE_OFFSET
	lda (ptr1),y		; child->p_pag[0]
	sta $FE03		; 6000-7FFF
	lda U_DATA__U_PAGE
	sta $FE02		; 4000-5FFF

	; Now use that window to copy 56K from 0000-DFFF

	jsr bank2bank		; copies 8K
	; This assumes our MMU is r/w. We'd need shadows otherwise
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
	inc $FE02		; next 8K
	inc $FE03
	jsr bank2bank		; copies 8K
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

_create_init_common:
	lda #$00
	sta $FE02
	lda #$08
	sta $FE03
	jsr bank2bank
	jmp map_kernel
;
;	The switch proc pointer cannot live anywhere in common as we switch
;	common on process switch
;
	.data

switch_proc_ptr: .word 0
