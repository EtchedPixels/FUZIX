#
!
!	Low level platform support for v8080
!

	#include "../kernel-8080.def"

.sect .common

.define _platform_monitor
.define _platform_reboot

_platform_monitor:
_platform_reboot:
	di
	hlt

.define platform_interrupt_all

platform_interrupt_all:
	ret

.sect .code

.define init_early

init_early:
	ret

.define init_hardware

init_hardware:
	! Hack for now
	lxi h,256
	shld _ramsize
	lxi h,192
	shld _procmem

	mvi a,1
	out 8			! Timer on

	jmp _program_vectors_k
	

.sect .common

.define _int_disabled
_int_disabled:
	.data1 1

.define _program_vectors

_program_vectors:
	di
	pop d
	pop h
	push h
	push d
	call map_process
	call _program_vectors_k
	call map_kernel_di
	ret

_program_vectors_k:
	mvi a,0xc3
	sta 0
	sta 0x30
	sta 0x66
	lxi h,null_handler
	shld 1
	lxi h,unix_syscall_entry
	shld 0x31
	lxi h,nmi_handler
	shld 0x67
	ret

!
!	Memory mapping
!
.define map_kernel
.define map_kernel_di

map_kernel:
map_kernel_di:
	push psw
	xra a
	out 0x40
	pop psw
	ret

.define map_process
.define map_process_di
.define map_process_a

map_process:
map_process_di:
	mov a,h
	ora l
	jz map_kernel
	mov a,m
map_process_a:
	out 0x40
	ret

.define map_process_always
.define map_process_always_di

map_process_always:
map_process_always_di:
	push psw
	lda U_DATA__U_PAGE
	out 0x40
	pop psw
	ret

.define map_save_kernel

map_save_kernel:
	push psw
	in 0x40
	sta map_save
	xra a
	out 0x40
	pop psw
	ret

.define map_restore

map_restore:
	push psw
	lda map_save
	out 0x40
	pop psw
	ret

map_save:
	.data1 0

.define outchar

outchar:
	push psw
outcharw:
	in 0
	rar
	jnc outcharw
	pop psw
	out 0
	ret

