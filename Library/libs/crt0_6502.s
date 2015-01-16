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
	.import		popax
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
	jsr	___stdio_init_vars
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

	jmp	_exit		; exit syscall, AX holds our return code
				; for a fastcall return to nowhere.

initmainargs:			; Hardcoded compiler dumbness
	rts

	.bss
_environ:	.word	0
