;
; Startup code for cc65 on FUZIX
;
; Based on code by Debrune Jérôme <jede@oric.org>
; and Ullrich von Bassewitz <uz@cc65.org> 2014-08-22, Greg King
;

        .export         _exit
        .import         initlib, donelib
	.import		__CODE_SIZE__, __RODATA_SIZE__
	.import		__DATA_SIZE__, __BSS_SIZE__
	.import		__exit
	.import		_environ
	.import		_main

        .include        "zeropage.inc"

; Place the startup code in a special segment.

.segment        "STARTUP"
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
; Call the module constructors.
        jsr     initlib

; Push the command-line arguments; and, call main().
;
; Need to save the environment ptr. The rest of the stack should be
; fine.
;
	lda	sp
	ldx	sp+1
	clc
	adc	#4
	bcc	l1
	inx
l1:	sta	_environ
	stx	_environ+1
	ldy	#4
        jsr     _main

; Call the module destructors. This is also the exit() entry.

_exit:  pha
	txa
	pha
	jsr     donelib         ; Run module destructors
	pla
	tax
	pla
	jmp	__exit		; exit syscall, AX holds our return code
				; for a fastcall return to nowhere.