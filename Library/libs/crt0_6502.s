;
; Startup code for cc65 on FUZIX
;
; Based on code by Debrune Jérôme <jede@oric.org>
; and Ullrich von Bassewitz <uz@cc65.org> 2014-08-22, Greg King
;

	.import		__CODE_SIZE__, __RODATA_SIZE__
	.import		__DATA_SIZE__, __BSS_SIZE__
	.import		__STARTUP_SIZE__
	.import		_exit
	.export		_environ
	.export		initmainargs
	.export		__syscall_hook
	.import		_main
	.import		popax, pushax
	.import		___stdio_init_vars
	.import		decsp8, incsp8, decsp2, incsp2
	.import		jmpvec

	.export __STARTUP__ : absolute = 1;

        .include        "zeropage.inc"

.segment "STARTUP"

__syscall_hook:				; Stubs overlay this
head:
	.word 	$80A8
	.byte	3			; 6502 family
	.byte	0			; 6502 (we don't yet use 65C02 ops)
	.byte	>head			; Load address page
	.byte	0			; No hint bits
	.word	__CODE_SIZE__ + __RODATA_SIZE__ + __STARTUP_SIZE__
	.word	__DATA_SIZE__
	.word	__BSS_SIZE__
	.byte 	<start			; Offset from load page as entry
	.byte	0			; No size hint
	.byte	0			; No stack hint
	.byte	0			; TODO - ZP size

	.word	__sighandler		; IRQ path signal handling helper

;
;	First cut at sighandling stubs, horrible on 6502!
;
;	The user stack may be invalid. If we hit mid update then the low
;	byte may be updated but the carry into the high byte not done.
;
;	I think therefore that providing we dec sp+1 we are always ok and
;	will be somewhere between 0 and 255 bytes of valid
;
;	We save only the minimal scratch variables. The logic here is that
;	this routine will not touch the 'register' variables in ZP, but
;	any C function it calls will treat them as callee save so will do
;	any needed save/restore for us. So we need to save ptr1-4, tmp1-4
;	and jmpvec
;
__sighandler:
	dec	sp+1		; ensure we are safe C stack wise
	lda	jmpvec+1
	ldx	jmpvec+2
	jsr 	pushax
	jsr	decsp8
	jsr	decsp8
	jsr	decsp2
	jsr	stash_zp	; saves sp etc
	pla
	sta	jmpvec+1	; ZP was swapped (as was jmpvec)
	pla
	sta	jmpvec+2	; Patch jmpvec
	pla

	ldx	#0		; signal(sig)
	jsr	jmpvec		; no jsr (x) so fake it
	jsr	stash_zp	; recovers everything
	jsr	incsp2
	jsr	incsp8
	jsr	incsp8
	jsr	popax
	stx	jmpvec+2
	sta	jmpvec+1
	inc	sp+1		; back to old stack
initmainargs:			; Hardcoded compiler dumbness
	rts			; will return via the kernel stub
;
;
;	On entry sp/sp+1 have been set up by the kernel (this has to be
;	done in kernel as we might take a signal very early on). Above the
;	sp are the argument vectors, environment etc. In other words we
;	are basically a straight function call from the kernel stub.
;
;	Our image has been loaded into RAM, any spare memory has been zeroed
;	for us and we are ready to roll.
;
start:
; Push the command-line arguments; and, call main().
;
; Need to save the environment ptr. The rest of the stack should be
; fine.
;
;
;	FIXME: sort out some sort of constructor mechanism for this
;
	jsr	___stdio_init_vars
	lda	sp
	ldx	sp+1
	clc
	adc	#4
	bcc	l1
	inx
l1:	sta	_environ
	stx	_environ+1
;	jsr	popax		; Pull argv off the stack leaving argc
	ldy	#2		; 2 bytes of args
        jsr     _main

; Call the module destructors. This is also the exit() entry.

	jmp	_exit		; exit cleanup, AX holds our return code
				; for a fastcall return to nowhere.

;
; Swap the C temporaries - smaller than separate save/loaders
;
stash_zp:
	ldy	#0
stash_loop:
	lda	(sp),y		; swap stack offset
	pha
	ldx	sp+2,y		; and ZP variable
	txa
	sta	(sp),y
	pla
	tax
	stx	sp+2,y
	iny
	cpy	#zpsavespace - 2	; 18 bytes
	bne	stash_loop
	rts


;
;	Ensure this version is found before the broken one in cc65 runtime
;	whichh won't work on 65C816

;
; Ullrich von Bassewitz, 06.08.1998
;
; CC65 runtime: call function via pointer in ax
; Reworked Alan Cox 2017 to handle cases where ZP != 0
;

        .export         callax
	.import		jmpvec

	.macpack	generic
	.macpack	cpu

callax:
	sta     jmpvec+1
        stx     jmpvec+2
        jmp     (jmpvec+1)         ; jump there



	.bss
_environ:	.res	2
oldsp:		.res	2
