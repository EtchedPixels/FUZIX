	.text
	.globl _program_vectors
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar
	.globl init_hardware
	.globl init_early

init_early:
_program_vectors:
	rts pc
	
_plt_monitor:
_plt_reboot:
	jmp _plt_reboot

/*	.data FOR now */
	.even
	.globl _udata

_udata:
	.= . + 512


	.text

/*
 *	KL11 and equivalents BR 4  vec 064 (tx) 060 (rx)
 */ 

outchar:
	bitb $0x80,0177564		/* test bit 7 of TCSR (tx ready) */
	beq outchar
	mov r0,0177566
	rts pc

init_hardware:
	/* FIXME: set up traps */
	/* FIXME: make up some numbers for the moment */
	mov $512,r0
	mov r0,_ramsize
	sub $64,r0
	mov r0,_procmem
	/* KW11: Enable IRQ at 0100 every line frequency tick BR 6 vec 100 */
	mov $0x4000,0177546
	/* Polling for now on the console */
	clr 0177560
	rts pc

