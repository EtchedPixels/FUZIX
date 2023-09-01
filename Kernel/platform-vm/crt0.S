#include "../kernel-vm.def"

	.code

	.export init

	; VM vectors
	.word	init
	.word	0
	.word	0

init:
	constuc 0
        call irq
	const kstack_top
	setsp

	const __bss_size
	push
	constuc 0
	push
	const __bss
	push
	call  __memset
	spmod8 6

	call init_hardware
	call _fuzix_main
	constuc 0
	call irq
stop:   call idle
        jump stop
