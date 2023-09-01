;
;	Low level platform support for virtual
;

	#include "../kernel-vm.def"

	.common

	.export _plt_monitor
	.export _plt_reboot

_plt_monitor:
_plt_reboot:
	z80call __bytecode
	call sysexit

	.export plt_interrupt_all

	.code

	.export init_early

init_early:
	z80call __bytecode
	exit

	.common


	.export init_hardware


	.export _int_disabled
_int_disabled:
	.byte 1

	.export _program_vectors

_program_vectors:
	z80call __bytecode
	constuc 1
	call irq
	push
	loadl8 4
	deref
	push
	call map_process
	call _program_v_u
	call map_kernel_di
	pop
	call irq
	exit

init_hardware:
_program_v_k:
_program_v_u:
	z80call __bytecode
	const interrupt_handler
	push
	constuc 4			; VM timer interrupt vector
	assign
	exit

;
;	Memory mapping
;
	.export map_kernel
	.export map_kernel_di

map_kernel:
map_kernel_di:
	z80call	__bytecode
	push
	constuc 0
	call setbank_a
	pop
	exit

	.export map_process
	.export map_process_di
	.export map_proc_a

map_process:
map_process_di:
	z80call __bytecode
	jfalse map_kernel
	deref
	push
	call setbank_a
	exit

map_proc_a:
	z80call __bytecode
	push
	call setbank_a
	pop
	exit
	

	.export map_proc_always_di
	.export map_proc_always

map_proc_always:
map_proc_always_di:
	z80call __bytecode
	push
	const U_DATA__U_PAGE
	deref
	call setbank_a
	pop
	exit

	.export map_save_kernel

map_save_kernel:
	z80call __bytecode
	push
	constuc	0
	call setbank_a
	push
	const map_save
	assign
	pop
	exit

	.export map_restore

map_restore:
	z80call __bytecode
	push
	const map_save
	deref
	call setbank_a
	pop
	exit

map_save:
	.word 0
