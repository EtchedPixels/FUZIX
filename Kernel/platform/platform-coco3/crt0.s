;;;
;;; The Kernel C run-time / start routine
;;;

;;;  imported symbols
	.globl 	_fuzix_main
	.globl 	init_early
	.globl 	init_hardware
	.globl 	kstack_top


	;; exported symbols
	.globl	start

	;; startup code @0
	.area 	.start
	jmp	start

	.area 	.text

start:	orcc 	#0x10		; interrupts definitely off
	lds 	#kstack_top

	;; zero out kernel's bss section
	ldx 	#__sectionbase_.bss__
	ldy 	#__sectionlen_.bss__
bss_wipe:
	clr 	,x+
	leay 	-1,y
	bne 	bss_wipe

	jsr 	init_early
	jsr 	init_hardware
	jsr 	_fuzix_main
	orcc 	#0x10
stop:	bra 	stop
