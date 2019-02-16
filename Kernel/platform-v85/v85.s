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
	mvi a,0x50
	out 0xFE
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
	lxi h,64		! 1 * 48K + 16K common
	shld _ramsize
	lxi h,0
	shld _procmem

	mvi a,0x40
	out 0xFE		! 10Hz timer on

	mvi a,0x96		! Intialize the ACIA
	out 0

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
!
!	The 8085 has some vectors of its own
!	0x24 is NMI
!	0x2C/0x34/0x3C are vectored interrupts
!	This also means we need a customised .rst handler to leave room for
!	NMI and RST 5.5 ideally.
!
!	We have the ACIA on 0x3C (polled for the moment) and the timer
!	on 0x34
!
_program_vectors_u:
	mvi a,0xc3
	sta 0
	lxi h,null_handler
	shld 1
!
!	FIXME: rst code fouls NMI
!	
!	sta 0x24
!	lxi h,nmi_handler
!	shld 0x25
	sta 0x30
	lxi h,unix_syscall_entry
	shld 0x31
	sta 0x3c
	lxi h,acia_interrupt
	shld 0x3d
	sta 0x34
	lxi h,interrupt_handler
	shld 0x35
	!
	!	Just in case
	!
	lxi h,unexpected
!	sta 0x2C
!	shld 0x2D
	sta 0x38
	shld 0x39
	ret

unexpected:
	lxi h,unexpect
	call outstring
	ei
	ret

unexpect:
	.ascii '[unexpected irq]'
	.data1 10,0
!
!	Memory mapping
!
.define map_kernel
.define map_kernel_di

map_kernel:
map_kernel_di:
map_buffers:
	push psw
	mvi a,1
	sta curmap
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
map_for_swap:
map_process_a:
	sta curmap
	out 0x40
	ret

.define map_process_always
.define map_process_always_di

map_process_always:
map_process_always_di:
	push psw
	lda U_DATA__U_PAGE
	sta curmap
	out 0x40
	pop psw
	ret

.define map_save_kernel

map_save_kernel:
	push psw
	lda curmap
	sta map_save
	mvi a,1
	sta curmap
	out 0x40
	pop psw
	ret

.define map_restore

map_restore:
	push psw
	lda map_save
	sta curmap
	out 0x40
	pop psw
	ret

.define _probe_bank

_probe_bank:
	pop d
	pop h
	push h
	push d
	mov a,l
	call map_process_a
	lxi d,-1
	lxi h,4
	mvi a,0x55
	mov m,a
	cmp m
	jnz nobank
	inr m
	inr a
	cmp m
	jnz nobank
	inx d
nobank:
	jmp map_kernel

map_save:
	.data1 0
curmap:
	.data1 1
.define outchar
.define _ttyout

!
!	6850 ACIA
!
_ttyout:
	pop h
	pop d
	push d
	push h
	mov a,e
outchar:
	push psw
outcharw:
	in 0
	ani 2
	jz outcharw
	pop psw
	out 1
	ret

.define _ttyready

_ttyready:
	in 0
	ani 2
	mov e,a
	ret

.define _acia_poll

!
!	Call with interrupts off
!
_acia_poll:
	lxi h,acia_queue+1
	mov a,m			! read pointer
	mov e,a			! save it
	dcx h			! queue pointer
	cmp m			! queue = read + 1 -> empty
	jz empty
	inr a
	inx h			! point back to read
	mov m,a			! move on one
	lxi h,acia_rxbuf	! data buffer
	mvi d,0
	dad d			! plus offset
	mov e,m			! return char in DE
	mvi d,0
	ret
empty:
	lxi d,0xffff		! No luck
	ret


!
!	ACIA interrupt
!
acia_interrupt:
	push psw
	in 0
	rar
	cc acia_rx		! we only do rx interrupts for now
	pop psw
	ei
	ret
acia_rx:
	push h
	push d
	lxi h,acia_queue
	mov a,m			! queue pointer
	mov e,a			! save it
	inx h			! move on to rx pointer
	inr a			! move on
	cmp m			! if we would hit rx pointer we are full
	jz acia_rx_over		! so bail out
	dcx h			! back to queue pointer
	mov m,a			! save new pointer
	mvi d,0
	lxi h,acia_rxbuf	! get data offset and save
	dad d			! could simplify if page aligned...
	in 1
	mov m,a
acia_rx_over:
	pop d
	pop h
	ret

acia_rxbuf:
	.space 256
acia_queue:
	.data1 1		! queue pointer
	.data1 0		! read pointer
	
!
!	IDE controller
!
.sect .common

.define _devide_readb
.define _devide_writeb

_devide_readb:
	pop h
	pop d
	push d
	push h
	mov a,e
	sta .patch1+1
.patch1:
	in 0
	mov e,a
	mvi d,0
	ret

_devide_writeb:
	lxi h,2
	dad sp
	mov a,m
	sta .patch2+1
	inx h
	inx h
	mov a,m
.patch2:
	out 0
	ret

.define _devide_read_data
.define _devide_write_data

_devide_read_data:
	push b
	lxi h,_blk_op
	mov e,m
	inx h
	mov d,m			! Address
	inx h
	mov a,m			! Mapping type
	cpi 2
	jnz not_swapin
	inx h			! Swap page
	mov a,m
	call map_for_swap
	jmp doread
not_swapin:
	ora a
	jz rd_kernel
	call map_process_always
	jmp doread
rd_kernel:
	call map_buffers
doread:
	mvi b,0
	xchg
readloop:
	in 0x10
	mov m,a
	inx h
	in 0x10
	mov m,a
	inx h
	inr b
	jnz readloop
	pop b
	jmp map_kernel

_devide_write_data:
	push b
	lxi h,_blk_op
	mov e,m
	inx h
	mov d,m
	inx h
	mov a,m
	cpi 2
	jnz not_swapout
	inx h
	mov a,m
	call map_for_swap
	jmp dowrite
not_swapout:
	ora a
	jz wr_kernel
	call map_process_always
	jmp dowrite
wr_kernel:
	call map_buffers
dowrite:
	mvi b,0
	xchg
writeloop:
	mov a,m
	out 0x10
	inx h
	mov a,m
	out 0x10
	inx h
	inr b
	jnz writeloop
	pop b
	jmp map_kernel
