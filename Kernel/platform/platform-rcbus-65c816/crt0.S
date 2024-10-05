;	We end up first in the image
;

#include "kernel.def"

	.65c816
	.a16
	.i16

	.byte 0x02
	.byte 0x65


entry:
;
;	We are entered at 0x0202 just after the required magic number
;	At this point we are in .a16 .i16 the boot loader having put
;	us in the right CPU mode
;
;	Our CPU stack right now is at 0x0100
;
	sei			; interrupts off
	cld			; decimal off

	ldx	#_udata
	lda	#256
wipe_ud:
	stz	0,x
	inx
	inx
	dec	a
	bne	wipe_ud

	ldx	#__bss
	lda	#__bss_size
	lsr	a

wipe_b:
	stz	0,x
	inx
	inx
	dec	a
	bne	wipe_b

	sep	#0x20
	.a8
	lda	#__bss_size
	and	#1
	beq	no_tail
	stz	0,x
no_tail:
	rep	#0x20
	.a16
gogogo:
	; TODO: kernel v user split cpu stacks (here and core)
	ldy	#kstack_top
	lda	#KERNEL_DP
	pha
	pld	; Set up direct page for kernel into the upper area
		; TODO: fix up core code for kernel v user dp and s
	
	jsr init_early
	jsr init_hardware
	jsr _fuzix_main		; Should never return
	sei			; Spin
stop:	jmp stop

	; TODO vectors

vectable:
	.word	0		; unused
	.word	0		; unused
	.word	illegal_inst	; COP
	.word	trap_inst	; BRK
	.word	abort_inst	; ABORT
	.word	nmi_handler	; NMI
	.word	0		; Unused (native reset)
	.word	interrupt_handler

	;
	;	Emulation mode vectors. If called badness occurred
	;
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation


