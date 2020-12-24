	.mri	1

start:	move	#$2700,sr	; Disable interrupts

	move.w	#$600,a0
	move.l	#kernel_begin,a1
	move.w	#(kernel_end-kernel_begin+3)/4,d0
install:
	move.l	(a1)+,(a0)+
	dbra	d0,install

	jmp	$600

	even
kernel_begin:
	incbin	"../fuzix.bin"
kernel_end:

	section .filename
	asciz	"FUZIX.TOS"
