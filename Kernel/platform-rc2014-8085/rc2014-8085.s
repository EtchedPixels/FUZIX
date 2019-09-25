#
!
!	Low level platform support for v8080
!

	#include "../kernel-8080.def"

#define ACIA_RESET	0x03
#define ACIA_RTS_LOW	0x96

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

.define _platform_idle

!
!	We are entirely interrupt driven
!
_platform_idle:
	hlt
	ret

.define _tms_interrupt

_tms_interrupt:
	in 0x99
	ani 0x80
	mov e,a
	mvi d,0
	ret

.sect .common


.define init_hardware

init_hardware:
	! 

	lxi h,512		! 1 * 48K + 16K common
	shld _ramsize
	lxi h,432
	shld _procmem

	lxi h,tmsinitdata
	mvi d,0x80
tmsinit:
	mvi a,0x88
	cmp d
	jz tmsdone
	mov a,d
	out 0x99
	mov a,m
	out 0x99
	inx h
	inr d
	jmp tmsinit

tmsdone:
	lxi h,0x8000
tmswait:
	dcx h
	jk no_tms

	! Now see if the device is actually present
	in 0x99
	rlc
	jnc tmswait
	!
	! The read should have cleared the interrupt, and we will run well
	! before the next one
	!
	in 0x99
	rlc
	jc tmswait
	!
	! TMS9918A is present
	!
	mvi a,1
	sta _tms9918a_present
	jmp uart_config
	!
	!	We still need a clock. Pray we have a 82C54 card at 0x3C
	!
	!	Channel 0 is clocked at the CPU clock and can output to a
	!	user pin (eg for serial clocking)
	!	Channel 1 is clocked at 1.8432MHz and drives
	!	Channel 2 out drives the interrupt line
	!
	!	All gate lines are pulled high
	!
no_tms:
	mvi a,0x76	! Counter 1, 16bit, mode 3 (square wave), not BCD
	out 0x3F
	mvi a,0x90	! Counter 2 , 8bit low, mode 0 (terminal count), not BCD
	out 0x3F
	mvi a,0xA6
	out 0x3D	! Counter 1 LSB		}	Generate a 100Hz
	mvi a,0x47	!			}	Square Wave into
	out 0x3D	! Counter 1 MSB		}	Counter 2
	mvi a,10
	out 0x3E	! Counter 2 LSB		}	Generate an interrupt
			!			}	at 10Hz

	!
	! See what is present. Look for 16550A and 68B50. We know that
	! it won't be a Z80 and we can't work the 16bit ports on the QUART
	!
uart_config:
	in 0xA0
	ani 2
	jz not_acia	! TX ready ought to be high...
	mvi a,ACIA_RESET
	out 0xA0
	in 0xA0
	ani 2
	jnz not_acia
	mvi a,1
	sta _acia_present
	mvi a,2
	out 0xA0
	mvi a,ACIA_RTS_LOW
	sta 0xA0

	!
	! Now check for a 16x50
	!
not_acia:
	in 0xC3
	mov b,a
	ori 0x80
	out 0xC3
	in 0xC1
	mov c,a
	mvi a,0xAA
	out 0xC1
	in 0xC1
	cpi 0xAA
	jnz no_uart
	mov a,b
	out 0xC3	! Flip back and should not see it
	in 0xC1
	cpi 0xAA
	jz  no_uart
	mov a,b
	ori 0x80
	out 0xC3
	mov a,c
	out 0xC1	! Restore the current baud rate
	mov a,b
	out 0xC3
	mvi a,1
	sta _uart_present
	!
	! Set up the interrupt on the uart
	mvi a,0x01
	out 0xC1	
	!
	! Into the C set up code
	!
no_uart:
	jmp _program_vectors_k

tmsinitdata:
	.data1 0x00
	.data1 0xD0
	.data1 0x00		! text at 0x0000 (room for 4 screens)
	.data1 0x00
	.data1 0x02		! patterns at 0x1000
	.data1 0x00
	.data1 0x00
	.data1 0xF1		! white on black

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
!	All the interrupts are wired to RST 6.5
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
!	The default behaviour for the supplied board is broken. This assumes
!	we are using the wiring patch to use RST6.5
!
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
.define _ttyout_uart
.define _ttyout_acia

!
!	16550A UART for now (should probe and pick)
!
_ttyout_uart:
	ldsi 2
	ldax d
outchar:
	push psw
outcharw:
	in 0xC5
	ani 0x20
	jz outcharw
	pop psw
	out 0xC0
	ret

_ttyout_acia:
	ldsi 2
	ldax d
	out 0xC1
	ret

.define _uart_ready
.define _acia_ready

_uart_ready:
	in 0xC5
	ani 0x20
	mvi d,0
	mov e,a
	ret

_acia_ready:
	in 0xA0
	ani 2
	mvi d,0
	mov e,a
	ret

.define _uart_poll
.define _acia_poll

_uart_poll:
	in 0xC5
	ani 0x01
	lxi d,-1
	rz
	in 0xC0
	mvi d,0
	mov e,a
	ret

_acia_poll:
	in 0xA0
	rar
	lxi d,-1
	rz
	in 0xA1
	mvi d,0
	mov e,a
	ret

.define _uart_setup

_uart_setup:
	ret

! We just pass this the control byte. It's easier than most devices
.define _acia_setup

_acia_setup:
	ldsi 2
	ldax d
	out 0xA0
	ret

!
!	Add interrupt queued serial once we have the basics debugged
!
	
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

!
!	82C54 helper
!
.define _timer_check

_timer_check:
	mvi d,0
	mvi a,0xE8		!	count/status are active low !
	out 0x3F		!	Latch status of counter 2
	in 0x3E			!	Read status
	rlc
	mov e,d			!	Return 0
	rnc			!	Output is low - so not us
	mvi a,10
	out 0x3E		!	Set the count back to 10
	mov e,a			!	Return nonzero
	ret			!	Was us
