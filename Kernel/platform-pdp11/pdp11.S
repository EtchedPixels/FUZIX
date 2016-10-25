	.text
	.globl _program_vectors
	.globl _trap_monitor
	.globl _trap_reboot
	.globl outchar
	.globl init_hardware
	.globl init_early

init_early:
_program_vectors:
	rts pc
	
_trap_monitor:
_trap_reboot:
	jmp _trap_reboot

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
	bit $0x8000,0177564		/* test bit 7 of TCSR (tx ready) */
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
	rts pc

