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
	mvi a,1
	out 29

.define platform_interrupt_all

platform_interrupt_all:
	ret

.sect .text

.define init_early

init_early:
	ret

.sect .common


.define init_hardware

init_hardware:
	mvi a,8
	out 20
	! Hack for now
	lxi h,400		! 8 * 48K + 16K
	shld _ramsize
	lxi h,336
	shld _procmem

	mvi a,1
	out 27			! 100Hz timer on

	jmp _program_vectors_k

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
	call _program_vectors_u
	call map_kernel_di
	ret

_program_vectors_k:
	push b
	call .rst_init
	pop b
_program_vectors_u:
	mvi a,0xc3
	sta 0
	sta 0x30
	sta 0x38
	sta 0x66
	lxi h,null_handler
	shld 1
	lxi h,unix_syscall_entry
	shld 0x31
	lxi h,interrupt_handler
	shld 0x39
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
	out 21
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
	out 21
	ret

.define map_process_always
.define map_process_always_di

map_process_always:
map_process_always_di:
	push psw
	lda U_DATA__U_PAGE
	out 21
	pop psw
	ret

.define map_save_kernel

map_save_kernel:
	push psw
	in 21
	sta map_save
	xra a
	out 21
	pop psw
	ret

.define map_restore

map_restore:
	push psw
	lda map_save
	out 21
	pop psw
	ret

map_save:
	.data1 0

.define outchar
.define _ttyout

!
!	Hack for Z80pack for now
!
_ttyout:
	pop h
	pop d
	push d
	push h
	mov a,e
outchar:
!	push psw
!outcharw:
!	in 0
!	ani 2
!	jz outcharw
!	pop psw
	out 1
	ret

.define _ttyout2

_ttyout2:
	pop h
	pop d
	push d
	push h
outw2:
	in 2
	ani 2
	jz outw2
	mov a,e
	out 3
	ret

.define _ttyready
.define _ttyready2

_ttyready:
	in 0
	ani 2
	mov e,a
	ret
_ttyready2:
	in 2
	ani 2
	mov e,a
	ret

.define _tty_pollirq

_tty_pollirq:
	in 0
	rar
	jnc poll2
	in 1
	mov e,a
	mvi d,0
	push d
	mvi e,1
	push d
	call _tty_inproc
	pop d
	pop d
poll2:
	in 40
	rar
	rnc
	in 41
	mov e,a
	mvi d,0
	push d
	mvi e,2
	push d
	call _tty_inproc
	pop d
	pop d
	ret


.sect .common

.define _fd_op

_fd_op:
	lxi h,_fd_drive
	mov a,m
	out 10			! drive
	inx h
	mov a,m
	out 11			! track
	inx h
	mov a,m
	out 12			! sector l
	inx h
	mov a,m
	out 17			! sector h
	inx h
	mov a,m
	out 15			! dma l
	inx h
	mov a,m
	out 16			! dma h
	inx h
	mov a,m
	out 21			! mapping
	inx h
	mov a,m
	out 13			! issue
	xra a
	out 21			! kernel mapping back
	in 14			! return status
	mov e,a
	mvi d,0
	ret

.define _fd_drive
.define _fd_track
.define _fd_sector
.define _fd_dma
.define _fd_page
.define _fd_cmd

_fd_drive:
	.data1 0
_fd_track:
	.data1 0
_fd_sector:
	.data2 0
_fd_dma:
	.data2 0
_fd_page:
	.data1 0
_fd_cmd:
	.data1 0
