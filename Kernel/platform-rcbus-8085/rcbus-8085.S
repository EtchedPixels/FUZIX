;
;	Low level platform support for rcbus-8085
;

	#include "../kernel-8085.def"

#define ACIA_RESET	0x03
#define ACIA_RTS_LOW	0x96

	.setcpu 8085
	.common

.export bankid

	; This is filled in at boot in each bank with the bank id for
	; this common + kernel map
bankid:
	.byte 0

.export _plt_monitor
.export _plt_reboot

_plt_monitor:
_plt_reboot:
	di
	mvi a,1			; Map ROM back in low
	out 0xFF
	rst 0

.export plt_interrupt_all

plt_interrupt_all:
	ret


	.code

.export init_early

init_early:
	ret

.export _plt_idle

;
;	We are entirely interrupt driven
;
_plt_idle:
	hlt
	ret

.export _tms_interrupt

_tms_interrupt:
	in 0x99
	ani 0x80
	mov l,a
	mvi h,0
	ret

	.common

.export init_hardware

init_hardware:
	lxi h,512		; 1 * 48K + 16K common
	shld _ramsize
	lxi h,432
	shld _procmem

	lxi h,tmsinitdata
	mvi d,0x80
tmsinit:
	mvi a,0x88
	cmp d
	jz tmsdone
	mov a,m
	out 0x99
	mov a,d
	out 0x99
	inx h
	inr d
	jmp tmsinit

tmsdone:
	lxi h,0x8000
tmswait:
	dcx h
	jk no_tms

	; Now see if the device is actually present
	in 0x99
	rlc
	jnc tmswait
	;
	; The read should have cleared the interrupt, and we will run well
	; before the next one
	;
	in 0x99
	rlc
	jc tmswait
	;
	; TMS9918A is present
	;
	mvi a,1
	sta _tms9918a_present
	;
	;	Turn interrupts on
	;
	call _vdp_init
	call _vdp_load_font
	call _vdp_restore_font
	jmp uart_config
	;
	;	We still need a clock. Pray we have a 82C54 card at 0x3C
	;
	;	Channel 0 is clocked at the CPU clock and can output to a
	;	user pin (eg for serial clocking)
	;	Channel 1 is clocked at 1.8432MHz and drives
	;	Channel 2 out drives the interrupt line
	;
	;	All gate lines are pulled high
	;
no_tms:
	mvi a,0x76	; Counter 1, 16bit, mode 3 (square wave), not BCD
	out 0x3F
	mvi a,0x90	; Counter 2 , 8bit low, mode 0 (terminal count), not BCD
	out 0x3F
	mvi a,0xA6
	out 0x3D	; Counter 1 LSB		}	Generate a 100Hz
	mvi a,0x47	;			}	Square Wave into
	out 0x3D	; Counter 1 MSB		}	Counter 2
	mvi a,10
	out 0x3E	; Counter 2 LSB		}	Generate an interrupt
			;			}	at 10Hz

	;
	; See what is present. Look for 16550A and 68B50. We know that
	; it won't be a Z80 and we can't work the 16bit ports on the QUART
	;
uart_config:
	in 0xA0
	ani 2
	jz not_acia	; TX ready ought to be high...
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

	;
	; Now check for a 16x50
	;
not_acia:
	in 0xC3		; Save the existing status
	ani 0x7F
	mov b,a
	ori 0x80	; Turn on access to the rate bytes
	out 0xC3
	in 0xC1		; Read and save part of the rate
	mov c,a
	mvi a,0xAA
	out 0xC1	; Force it to AA
	in 0xC1
	cpi 0xAA	; Check it is valid as AA
	jnz no_uart
	mov a,b
	out 0xC3	; Flip back and should not see it
	in 0xC1
	cpi 0xAA
	jz  no_uart
	mov a,b
	ori 0x80
	out 0xC3
	mov a,c
	out 0xC1	; Restore the current baud rate
	mov a,b
	out 0xC3
	mvi a,1
	sta _uart_present
	;
	; Set up the interrupt on the uart
	;
	mvi a,0x01
	out 0xC1	
	;
	; Into the C set up code
	;
no_uart:
	mov a,b
	out 0xC3

	call _program_kvectors
	;
	; We can now call C code
	;
	call _rctty_init	; Must run before fuzix_main
	ret

tmsinitdata:
	.byte 0x00
	.byte 0xD0
	.byte 0x00		; text at 0x0000 (room for 4 screens)
	.byte 0x00
	.byte 0x02		; patterns at 0x1000
	.byte 0x00
	.byte 0x00
	.byte 0xF1		; white on black

.export _int_disabled
_int_disabled:
	.byte 1

.export _program_vectors

_program_vectors:
	di
	ldsi 2
	lhlx
	call map_proc
	call _program_uvectors
	call map_kernel_di
	ret

_program_kvectors:
;
;	The 8085 has some vectors of its own
;	0x24 is NMI
;	0x2C/0x34/0x3C are vectored interrupts
;	This also means we need a customised .rst handler to leave room for
;	NMI and RST 5.5 ideally.
;
;	All the interrupts are wired to RST 6.5
;
_program_uvectors:
	mvi a,0xc3
	sta 0
	lxi h,null_handler
	shld 1
;
;	FIXME: rst code fouls NMI
;
;	sta 0x24
;	lxi h,nmi_handler
;	shld 0x25
	sta 0x30
	lxi h,unix_syscall_entry
	shld 0x31
;
;	We use RST 6.5, this is correct for the EP 80C85 board. The original
;	8085 design used INT which doesn't work as 8085 INT requires a
;	vector, so the board needs a wire mod
;
	sta 0x34
	lxi h,interrupt_handler
	shld 0x35
	;
	;	Just in case
	;
	lxi h,unexpected
;	sta 0x2C
;	shld 0x2D
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
	.byte 10,0

;
;	Memory mapping
;
.export map_kernel
.export map_kernel_di

map_kernel:
map_kernel_di:
map_buffers:
	push psw
	lda bankid		; Get the ID of the current common bank
kmap:
	DI
	out 0xFF
	sta curmap
	; Can check this as we are in kernel mapping
	lda _int_disabled
	ora a
	jnz no_ei
	EI
no_ei:
	pop psw
	ret

.export map_proc
.export map_proc_di
.export map_proc_a
.export map_for_swap

map_proc:
map_proc_di:
	mov a,h
	ora l
	jz map_kernel
	mov a,m
map_for_swap:
map_proc_a:
	; Save map into kernel map (our map was kernel if this is called)
	sta curmap
	out 0xFF		; No race risk this direction
	ret

.export map_proc_always
.export map_proc_always_di

map_proc_always:
map_proc_always_di:
	push psw
	lda U_DATA__U_PAGE
setmap:
	sta curmap		; As with map_proc
	out 0xFF
	pop psw
	ret

.export map_save_kernel

; Only called during an interrupt
map_save_kernel:
	push psw
	lda bankid
	out 0xFF		; into kernel bank
	lda curmap
	sta map_save		; save the old mapo
	lda bankid
	sta curmap		; save the new map correctly
	pop psw
	ret

.export map_restore

; We are always returning from kernel in this case.
map_restore:
	push psw
	lda map_save		; We have kenrel mapped
	sta curmap
	out 0xFF		; No race
	pop psw
	ret


; We keep these in the kernel map

	.data

.export curmap

map_save:
	.byte 0
curmap:
	.byte 1

	.code

.export outchar
.export _ttyout_uart
.export _ttyout_acia

;
;	16550A UART for now with outchar debug (FIXME: pick from probe)
;
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
	out 0xA1
	ret

.export _uart_ready
.export _acia_ready

_uart_ready:
	in 0xC5
	ani 0x20
	mvi h,0
	mov l,a
	ret

_acia_ready:
	in 0xA0
	ani 2
	mvi h,0
	mov l,a
	ret

.export _uart_poll
.export _acia_poll

_uart_poll:
	in 0xC5
	ani 0x01
	lxi h,-1
	rz
	in 0xC0
	mvi h,0
	mov l,a
	ret

_acia_poll:
	in 0xA0
	rar
	lxi h,-1
	rz
	in 0xA1
	mvi h,0
	mov l,a
	ret

.export _uart_setup

_uart_setup:
	ret

; We just pass this the control byte. It's easier than most devices
.export _acia_setup

_acia_setup:
	ldsi 2
	ldax d
	out 0xA0
	ret

;
;	Add interrupt queued serial once we have the basics debugged
;
	
;
;	82C54 helper
;
.export _timer_check

_timer_check:
	mvi h,0
	mvi a,0xE8		;	count/status are active low ;
	out 0x3F		;	Latch status of counter 2
	in 0x3E			;	Read status
	rlc
	mov e,d			;	Return 0
	rnc			;	Output is low - so not us
	mvi a,10
	out 0x3E		;	Set the count back to 10
	mov l,a			;	Return nonzero
	ret			;	Was us

;
;	Joysticks
;

.export	_jsin1

_jsin1:
	mvi h,0
	in 0x01
	mov l,a
	ret

.export _jsin2

_jsin2:
	mvi h,0
	in 0x02
	mov l,a
	ret

;
;	USB adapter
;
.export _ch375_rdata

_ch375_rdata:
	in CH375_DATA
	mov l,a
	ret

.export _ch375_rstatus

_ch375_rstatus:
	in CH375_STATUS
	mov l,a
	ret

.export _ch375_wcmd

_ch375_wcmd:
	pop h
	pop d
	mov a,e
	out CH375_CMD
	push d
	pchl

.export _ch375_wdata

_ch375_wdata:
	pop h
	pop d
	mov a,e
	out CH375_DATA
	push d
	pchl

.export _nap20
;
;	Worsr case is an 8MHz Tundra 80C85 @ 8 blocks per uS so 160 clocks
;	of which we used 28 in the call/ret (so 132 left)
;
_nap20:
	mvi a,10	; Gives us 133 clocks in the loop, 161 clocks total
nap:
	dcr a		; 4	(14 per loop + 7 on last)
	jnz nap		; 10/7
	ret

	.common

.export _ch375_rblock

; These move 64 bytes at a time as the interface runs on USB URBs not disk
; blocks
_ch375_rblock:
	pop d
	pop h
	push h
	push d
	push b		; Save BC required by ABI
	lda _td_raw
	ora a
	jz rblock_k
	dcr a
	jz rblock_u
	lda _td_page
	call map_for_swap
	jmp rblock_k
rblock_u:
	call map_proc_always
rblock_k:
	mvi b,32
rblock:
	in CH375_DATA
	mov m,a
	inx h
	in CH375_DATA
	mov m,a
	inx h
	dcr b
	jnz rblock
	pop b
	jmp map_kernel


.export _ch375_wblock

_ch375_wblock:
	pop d
	pop h
	push h
	push d
	push b		; Save BC required by ABI
	lda _td_raw
	ora a
	jz wblock_k
	dcr a
	jz wblock_u
	lda _td_page
	call map_for_swap
	jmp wblock_k
wblock_u:
	call map_proc_always
wblock_k:
	mvi b,32
wblock:
	mov a,m
	out CH375_DATA
	inx h
	mov a,m
	out CH375_DATA
	inx h
	dcr b
	jnz wblock
	pop b
	jmp map_kernel

