;
; Startup code for cc65 on FUZIX
;
; Based on code by Debrune Jérôme <jede@oric.org>
; and Ullrich von Bassewitz <uz@cc65.org> 2014-08-22, Greg King
;

	.import		__CODE_SIZE__, __RODATA_SIZE__
	.import		__DATA_SIZE__, __BSS_SIZE__
	.import		_exit
	.export		_environ
	.export		initmainargs
	.import		_main
	.import		popax, pushax
	.import		___stdio_init_vars

	.export __STARTUP__ : absolute = 1;

        .include        "zeropage.inc"

.segment "STARTUP"

	jmp	start

	.byte	'F'
	.byte	'Z'
	.byte	'X'
	.byte	'1'
	.byte	$20
	.word	0
	.word	__CODE_SIZE__ + __RODATA_SIZE__
	.word	__DATA_SIZE__
	.word	__BSS_SIZE__
	.word	0
	.word   0	; padding
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
;
__sighandler:
	jsr	stash_zp	; saves sp etc
	dec	sp+1		; ensure we are safe C stack wise
	pla
	sta	ptr1		; ZP was swapped
	pla
	sta	ptr1+1		; ptr1 is now the function
	pla
	ldx	#0
	jsr	pushax		; signal(int sig)
	jsr	jptr1		; no jsr (x) so fake it
	jsr	stash_zp	; recovers sp
initmainargs:			; Hardcoded compiler dumbness
	rts			; will return via the kernel stub
				; to user space
jptr1:	jmp	(ptr1)
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
	lda	sp
	ldx	sp+1
	clc
	adc	#4
	bcc	l1
	inx
l1:	sta	_environ
	stx	_environ+1
	jsr	popax		; Pull argv off the stack leaving argc
	ldy	#2		; 2 bytes of args
        jsr     _main

; Call the module destructors. This is also the exit() entry.

	jmp	_exit		; exit cleanup, AX holds our return code
				; for a fastcall return to nowhere.

;
;	The following is taken from the debugger example as referenced in
;	the compiler documentation. We swap a stashed ZP in our commondata
;	with an IRQ handler one. The commondata is per process and we depend
;	upon this to make it all work
;
; Swap the C temporaries
;
stash_zp:
        ldy     #zpsavespace-1
Swap1:  ldx     CTemp,y
        lda     <sp,y
        sta     CTemp,y
        txa
        sta     sp,y
        dey
        bpl     Swap1
        rts


	.bss
_environ:	.res	2
oldsp:		.res	2
CTemp:
		.res    2               ; sp
		.res    2               ; sreg
		.res    (zpsavespace-4) ; Other stuff
