;
;	Low level platform support for v8080
;

	#include "../../cpu-8080/kernel-8080.def"

	.common

	.export _plt_monitor
	.export _plt_reboot

_plt_monitor:
_plt_reboot:
	mvi a,1
	out 29

	.export plt_interrupt_all

plt_interrupt_all:
	ret

	.code

	.export init_early

init_early:
	ret

	.common


	.export init_hardware

init_hardware:
	mvi a,8
	out 20
	; Hack for now
	lxi h,400		; 8 * 48K + 16K
	shld _ramsize
	lxi h,336
	shld _procmem

	mvi a,1
	out 27			; 100Hz timer on

	jmp _program_v_k

	.export _int_disabled
_int_disabled:
	.byte 1

	.export _program_vectors

_program_vectors:
	di
	pop d
	pop h
	push h
	push d
	call map_proc
	call _program_v_u
	call map_kernel_di
	ret

_program_v_k:
	push b
;	call .rst_init
	pop b
_program_v_u:
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

;
;	Memory mapping
;
	.export map_kernel
	.export map_kernel_di

map_kernel:
map_kernel_di:
	push psw
	xra a
	out 21
	pop psw
	ret

	.export map_proc
	.export map_proc_di
	.export map_proc_a

map_proc:
map_proc_di:
	mov a,h
	ora l
	jz map_kernel
	mov a,m
map_proc_a:
	out 21
	ret

	.export map_proc_always_di
	.export map_proc_always

map_proc_always:
map_proc_always_di:
	push psw
	lda U_DATA__U_PAGE
	out 21
	pop psw
	ret

	.export map_save_kernel

map_save_kernel:
	push psw
	in 21
	sta map_save
	xra a
	out 21
	pop psw
	ret

	.export map_restore

map_restore:
	push psw
	lda map_save
	out 21
	pop psw
	ret

map_save:
	.byte 0

	.export outchar
	.export _ttyout

;
;	Hack for Z80pack for now
;
_ttyout:
	pop h
	pop d
	push d
	push h
	mov a,e
outchar:
;	push psw
;outcharw:
;	in 0
;	ani 2
;	jz outcharw
;	pop psw
	out 1
	ret

	.export _ttyout2

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

	.export _ttyready
	.export _ttyready2

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

	.export _tty_pollirq

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


	.common

	.export _fd_bankcmd
;
;	We need the right bank present when we cause the transfer
;
_fd_bankcmd:
	lxi	h,5
	dad	sp
	mov	d,m
	dcx	h
	mov 	e,m		; page ptr
	dcx	h
	dcx	h
	mov	l,m		; command

	xchg		; bank to HL command to E

	lda	_int_disabled
	di
	push	psw		; save DI state
	call	map_proc_di	; HL alread holds our bank
	mov	a, e		; issue the command
	out	13	;
	call	map_kernel_di	; return to kernel mapping
	pop	psw
	ora	a
	rnz
	ei
	ret
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.export _bufpool

	.buffers

_bufpool:
	.ds BUFSIZE * NBUFS
