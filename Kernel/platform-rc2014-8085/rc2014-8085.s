#
!
!	Low level platform support for v8080
!

	#include "../kernel-8080.def"

#define CTC_CH0		0x88
#define CTC_CH1		0x89
#define CTC_CH2		0x8A
#define CTC_CH3		0x8B

.sect .common

.define _platform_monitor
.define _platform_reboot

_platform_monitor:
_platform_reboot:
	!
	!	FIXME: needs work
	!
	xra a
	out 0x78		! Map ROM back in
	rst 0

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
	! 
	lxi h,512		! 1 * 48K + 16K common
	shld _ramsize
	lxi h,432
	shld _procmem

	! Quieten the CTC
	mvi a,0x43
	out CTC_CH0
	out CTC_CH1
	out CTC_CH2
	out CTC_CH3
	! Probe the CTC
	mvi a,0x47
	out CTC_CH2
	mvi a,0xAA
	out CTC_CH2
	in  CTC_CH2
	cpi 0xA9
	jnz no_ctc

	mvi a,0x07
	out CTC_CH2
	mvi a,0x02
	out CTC_CH2

	!
	!	We are now counting down from 2 very fast so we should
	!	only see those values if we read
	!
	mvi b,0
ctc_check:
	in CTC_CH2
	ani 0xFC
	jnz no_ctc
	dcr b
	jnz ctc_check
	!
	!	We apparently have a CTC
	!
	mvi a,1
	sta _ctc_present
	!
	!	Set it up (for now we assume RC2014 7.3Mhz but most people
	!	will be using a lower clock so fix this ?)
	!
	mvi a,0xB5
	out CTC_CH2
	mvi a,144
	out CTC_CH2	! 200Hz @ 7.3Mhz, 100Hz at 3.6
	!
	!	Channel 3 becomes the missed event counter
	!
	mvi a,0x47
	out CTC_CH3
	mvi a,0xFF
	out CTC_CH3

no_ctc:
	! Set up the interrupt on the uart
	mvi a,0x01
	out 0xC1	

	jmp _program_vectors_k

.define _int_disabled
_int_disabled:
	.data1 1

.define _program_vectors

_program_vectors:
	di
	ldsi 2
	lhlx
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
!	For the RC2014 system all interrupts go to the external interrupt
!	pin and none of the vectoring features and priority can be used 8(
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
!
!	The board is wired this way but I am not convinced that will
!	actually work out. If not we'll need to rewire the IRQ to RST 5.5
!
	sta 0x38
	lxi h,interrupt_handler
	shld 0x39
	!
	!	Just in case
	!
	lxi h,unexpected
!	sta 0x2C
!	shld 0x2D
	sta 0x34
	shld 0x35
	sta 0x3c
	shld 0x3d
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
	mvi a,32
	jmp setmap

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
	push psw
setmap:
	sta curmap
	out 0x78
	inr a
	out 0x79
	inr a
	out 0x7A
	pop psw
	ret

.define map_process_always
.define map_process_always_di

map_process_always:
map_process_always_di:
	push psw
	lda U_DATA__U_PAGE
	jmp setmap

.define map_save_kernel

map_save_kernel:
	push psw
	lda curmap
	sta map_save
	mvi a,32
	jmp setmap

.define map_restore

map_restore:
	push psw
	lda map_save
	jmp setmap

map_save:
	.data1 0
curmap:
	.data1 1
.define outchar
.define _ttyout

!
!	16550A UART for now (should probe and pick)
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
	in 0xC5
	ani 0x20
	jz outcharw
	pop psw
	out 0xC0
	ret

.define _uart_ready

_uart_ready:
	in 0xC5
	ani 0x20
	mov e,a
	ret

.define _uart_poll

_uart_poll:
	in 0xC5
	ani 0x01
	lxi d,-1
	rz
	in 0xC0
	mvi d,0
	mov e,a
	ret


.define _uart_setup

_uart_setup:
	ret

!
!	Add interrupt queued serial once we have the basics debugged
!

!
!	CTC small helper. Read the state of counter 3 (ticks elapsed) and
!	reset it
!
.define _ctc_check

_ctc_check:
	in CTC_CH3
	mov e,a
	mvi d,0
	mvi a,0x47
	out CTC_CH3
	mvi a,0xFF
	out CTC_CH3
	ret
	
!
!	IDE controller
!
.sect .common

.define _devide_readb
.define _devide_writeb

_devide_readb:
	ldsi 2
	ldax d
	sta .patch1+1
.patch1:
	in 0
	mov e,a
	mvi d,0
	ret

_devide_writeb:
	ldsi 2
	ldax d
	sta .patch2+1
	ldsi 4
	ldax d
.patch2:
	out 0
	ret

.define _devide_read_data
.define _devide_write_data

_devide_read_data:
	push b
	lxi d,_blk_op
	lhlx			! Address in HL
	xchg			! Address in DE, struct back in HL
	inx h
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
	lxi d,_blk_op
	lhlx
	xchg
	inx h
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

