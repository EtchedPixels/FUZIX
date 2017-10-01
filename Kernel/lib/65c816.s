;
;	The 65C816 memory management and switching logic is common
;	to all systems so keep it all here
;
        .include "../platform/kernel.def"
        .include "../kernel816.def"
	.include "../platform/zeropage.inc"

	.export _switchout
	.export _switchin
	.export _dofork

	.importzp ptr1
	.import _trap_monitor
	.import _chksigs
	.import _platform_idle
	.import _newproc
	.import _nready
	.import _inint
	.import _getproc
	.import _runticks
	.import outcharhex
	.import outstring


	.code
	.p816
	.i8
	.a8

_switchout:
	sei
	jsr	_chksigs
	rep	#$10			; Index to 16bit
	.i16
	ldx	#0
	phx
	ldx	sp
	phx
	tsx
	stx	U_DATA__U_SP
	sep	#$10			; Back to 8 for C code

	.i8
	lda	_nready
	bne	slow_path
idling:
	cli
	jsr	_platform_idle
	sei
	lda	_nready
	beq	idling
	cmp	#1
	bne	slow_path

	rep #$10
	.i16
	ldx	U_DATA__U_PTAB
	lda	0,x
	cmp	#P_READY
	bne	slow_path
	lda	#P_RUNNING
	sta	P_TAB__P_STATUS_OFFSET,x
	plx
	stx	sp
	plx				; discard 0
	sep	#$30
	; Get back into the way C expects us
	.i8
	.a8
	cli
	rts
slow_path:
	;
	;	Switch of task - save our udata and stack. Note we are
	;	saving the stack we are executing upon !
	;
	sep	#$30
	.i8
	.a8
	lda	U_DATA__U_PAGE
	sta	switch_patch_1+1		; target bank of save
	rep	#$30
	.i16
	.a16
	ldx	#U_DATA
	ldy	#U_DATA_STASH
	lda	#U_DATA__TOTALSIZE-1		; including our live stack
	phb
switch_patch_1:
	mvn	KERNEL_FAR,0		; save stack and udata
	plb
	sep #$30
	stz	_inint
	jsr	_getproc			; x,a holds process
	jsr	_switchin			; switch to process
	jsr	_trap_monitor			; bug out if it fails

;
;	FIXME: add swap support
;
_switchin:
	; not in theory needed
	sep	#$30
	.i8
	.a8

	sei
	sta	ptr1
	stx	ptr1+1

	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y			; bank of target

	; If this is zero we need swapping so the swapper checks go here
	; FIXME

	sta	switch_patch_2+2		; source bank of retrieve
	rep	#$30
	.i16
	.a16

	;	Set our stack pointer. We must not use it until the mvn
	;	is completed
	ldx	#U_DATA_STASH
	ldy	#U_DATA
	lda	#U_DATA__TOTALSIZE-1
switch_patch_2:
	;	FIXME check syntax required for bank value ??
	mvn	KERNEL_FAR,0
	;	after the MVN our data bank is KERNEL_DATA
	;	Our stack is now valid and we may use it again, our UDATA
	;	is for the new process
	ldx	U_DATA__U_SP			; correct stack pointer
	txs
	ldx	U_DATA__U_PTAB
	cpx	ptr1
	bne	switchinfail	;	wrong process !!
	stz	_runticks
	sep	#$20
	.a8
	lda	#P_RUNNING
	sta	P_TAB__P_STATUS_OFFSET,x
	;	This will only be needed once we swap, and we will need to
	;	do a few other fixups too
	lda	P_TAB__P_PAGE_OFFSET,x
	sta	U_DATA__U_PAGE
	plx	; stacked kernel space C sp
	stx	sp
	sep	#$10
	.i8
	lda	U_DATA__U_ININTERRUPT
	beq	notisr
	cli	; interrupts back on
notisr:
	pla	; return code
	plx
	rts
switchinfail:
	sep	#$30
	.a8
	.i8
	lda	ptr1+1
	jsr	outcharhex
	lda	ptr1
	jsr	outcharhex
        lda	#<badswitchmsg
	ldx	#>badswitchmsg
        jsr 	outstring
	; something went wrong and we didn't switch in what we asked for
        jmp	_trap_monitor
badswitchmsg:
	.byte	"_switchin: FAIL"
	.byte	13, 10, 0

	.a8
	.i8

_dofork:
	sta	ptr1			; new process ptr. U_DATA gives parent
	stx	ptr1+1
	lda	U_DATA__U_PAGE
	sta	fork_patch+2		; source bank (parent)
	sta	fork_patch_2+1		; destination udata stash
	asl	a
	adc	#STACK_BANKOFF
	sta	ptr2+1			; source for S and DP
	stz	ptr2
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y
	sta	fork_patch+1		; destination bank (child)
	asl	a
	adc	#STACK_BANKOFF		; find our S and DP banks as
					; those need copying too
	sta	ptr3+1			; dest for S and DP
	stz	ptr3

	rep	#$20
	.a16

	ldy	#P_TAB__P_PID_OFFSET	; Stack pid and sp
	lda	(ptr1),y
	pha				; Return value for parent
	rep	#$10
	.i16
	ldx	sp
	phx				; Saved user sp
	tsx
	stx	U_DATA__U_SP		; Stack pointer in udata

	; Our context is now a valid stack frame so we can save stuff
	ldx	#0
	txy
	lda	#MAP_SIZE	; 64K - udata shadow
	phb
fork_patch:
	mvn	0,0		; copy the entire bank below the save
	ldx	#U_DATA
	ldy	#U_DATA_STASH
	lda	#U_DATA__TOTALSIZE-1
fork_patch_2:
	mvn	KERNEL_FAR,0
	plb			; back to kernel bank

	ldx	ptr2
	ldy	ptr3
	lda	#$01FF		; DP and stack
	mvn	0,0

	;
	;	Final hairy detail - the child S value needs to be shifted
	;	versus parent so we restore it correctly
	;

	sep #$30
	.a8
	.i8
	lda	U_DATA__U_SYSCALL_SP+1
	sec
	sbc	ptr2+1
	clc
	adc	ptr3+1
	sta	U_DATA__U_SYSCALL_SP+1

	rep #$10
	.i16

	; At this point we have copied the parent into the child bank
	; and copied the current uarea into the child uarea
	plx			; discard frame we build for parent
	plx

	sep	#$30		; back to 8bit mode for C
	.a8
	.i8
	lda	ptr1
	ldx	ptr1+1
	jsr	_newproc
	; We are now being the child properly
	lda	#0
	sta	_runticks
	sta	_runticks+1
	tax			; return 0
	rts

