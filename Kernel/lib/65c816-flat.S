;
;	The 65C816 memory management and switching logic is common
;	to all systems so keep it all here
;
        .include "../platform/kernel.def"
        .include "../kernel816.def"
	.include "../platform/zeropage.inc"

	.export _plt_switchout
	.export _switchin
	.export _dofork

	.importzp ptr1
	.import _plt_monitor
	.import _plt_idle
	.import _makeproc
	.import _nready
	.import _inint
	.import _getproc
	.import _runticks
	.import outcharhex
	.import outstring
	.import kstack_base
	.import _udata
	.import pushax


	.code
	.p816
	.i8
	.a8

_plt_switchout:
	sei
	rep	#$10			; Index to 16bit
	.i16
	ldx	#0
	phx
	ldx	sp
	phx
	tsx
	stx	U_DATA__U_SP
	;
	;	Switch of task - save our udata and stack. Note we are
	;	saving the stack we are executing upon !
	;
	sep	#$30
	.i8
	.a8
	lda	U_DATA__U_PAGE
	sta	f:KERNEL_CODE_FAR+switch_patch_1+1		; target bank of save
.if ( KERNEL_BANK )
	sta	f:KERNEL_CODE_FAR+switch_patch_2+1		; target bank of save
.endif
	rep	#$30
	.i16
	.a16
	ldx	#U_DATA
	ldy	#U_DATA_STASH
	lda	#U_DATA__TOTALSIZE-1		; including our live stack
	phb
switch_patch_1:
	mvn	#KERNEL_BANK,#0		; save stack and udata

.if ( KERNEL_BANK )
	;
	;	Big (non bank 0) kernels by necessity split off the
	;	kstack into bank 0.
	;
	;	FIXME: some day split the user ZP to be half ZP half Kstack
	;
	lda	#$FF
	ldx	#kstack_base
switch_patch_2:
	mvn	#0,#0

.endif
	plb
	sep #$30
	stz	_inint
	jsr	_getproc			; x,a holds process
	jsr	_switchin			; switch to process
	jsr	_plt_monitor		; bug out if it fails

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

	sta	f:KERNEL_CODE_FAR+switch_patch_3+2	; source bank of retrieve
.if ( KERNEL_BANK )
	sta	f:KERNEL_CODE_FAR+switch_patch_4+2	; source bank of retrieve
.endif
	rep	#$30
	.i16
	.a16

	;	Set our stack pointer. We must not use it until the mvn
	;	is completed
	ldx	#U_DATA_STASH
	ldy	#U_DATA
	lda	#U_DATA__TOTALSIZE-1
switch_patch_3:
	mvn	#0,#KERNEL_BANK

.if ( KERNEL_BANK )
	;
	;	Big kernel - recover kstack into bank 0
	;
	ldy	#kstack_base
	lda	#$FF
switch_patch_4:
	mvn	#0,#0

.endif

	;	Bank may be invalid but we don't have a stack we can use
	;	so force a far access
	;	after this our data bank is KERNEL_DATA
	;	Our stack is now valid and we may use it again, our UDATA
	;	is for the new process
	lda	f:KERNEL_FAR+U_DATA__U_SP	; correct stack pointer
	tax
	txs
	;	Now our stack is valid fix the bank register

	sep	#$20
	.a8
	lda	#KERNEL_BANK
	pha
	plb

	ldx	U_DATA__U_PTAB
	cpx	ptr1
	bne	switchinfail	;	wrong process !!
	stz	_runticks

	lda	#P_RUNNING
	sta	a:P_TAB__P_STATUS_OFFSET,x
	;	This will only be needed once we swap, and we will need to
	;	do a few other fixups too
	lda	a:P_TAB__P_PAGE_OFFSET,x
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
        jmp	_plt_monitor
badswitchmsg:
	.byte	"_switchin: FAIL"
	.byte	13, 10, 0

	.a8
	.i8

_dofork:
	sta	ptr1			; new process ptr. U_DATA gives parent
	stx	ptr1+1
	lda	U_DATA__U_PAGE
	sta	f:KERNEL_CODE_FAR+fork_patch+2	 ; source bank (parent)
	sta	f:KERNEL_CODE_FAR+fork_patch_2+1 ; destination udata stash
.if ( KERNEL_BANK )
	sta	f:KERNEL_CODE_FAR+fork_patch_3+1 ; destination udata stash
.endif
	asl	a
	adc	#STACK_BANKOFF
	sta	ptr2+1			; source for S and DP
	stz	ptr2
	ldy	#P_TAB__P_PAGE_OFFSET
	lda	(ptr1),y
	sta	f:KERNEL_CODE_FAR+fork_patch+1	; destination bank (child)
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
	lda	#MAP_SIZE-1	; 64K - udata shadow
	phb
fork_patch:
	mvn	#0,#0		; copy the entire bank below the save
	ldx	#U_DATA
	ldy	#U_DATA_STASH
	lda	#U_DATA__TOTALSIZE-1
fork_patch_2:
	mvn	#KERNEL_BANK,#0
.if ( KERNEL_BANK )
	;
	;	For big kernels copy the kstack separately as it's in
	;	bank 0 but the C stack and udata are in kernel data.
	;
	ldx	#kstack_base
	lda	#$FF
fork_patch_3:
	mvn	#0,#0
.endif
	;
	;	Clone DP and stack between parent and child
	;
	ldx	ptr2		; in DP
	ldy	ptr3		; in DP
	lda	#$01FF		; DP and stack

	mvn	#0,#0
	plb			; back to corect bank register

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
	jsr	pushax
	lda	#<_udata
	ldx	#>_udata
	jsr	_makeproc
	; We are now being the child properly
	lda	#0
	sta	_runticks
	sta	_runticks+1
	tax			; return 0
	rts

