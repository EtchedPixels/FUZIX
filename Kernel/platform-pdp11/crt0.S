
		.globl __end
		.globl __bss_start

_start:
		bis 0340,0177776	/* IRQs off */
		mov $_udata+512,sp	/* Set kernel stack */
		mov $__bss_start,r0
1:					/* Wipe BSS */
		clr (r0)+
		cmp r0,$__end
		ble 1b
		jsr pc,init_early
		jsr pc,init_hardware
		jsr pc,_fuzix_main
		bis 0340,0177776
2:
		jmp 2b
