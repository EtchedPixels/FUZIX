
		.module bioshooks

		.globl _fuzixbios_init
		.globl _fuzixbios_reboot
		.globl _fuzixbios_monitor
		.globl _fuzixbios_getinfo
		.globl _fuzixbios_set_callbacks
		.globl _fuzixbios_param
		.globl _fuzixbios_idle
		.globl _fuzixbios_set_bank
		.globl _fuzixbios_init_done

		.globl _fuzixbios_serial_txready
		.globl _fuzixbios_serial_tx
		.globl _fuzixbios_serial_setup
		.globl _fuzixbios_serial_param
		.globl _fuzixbios_serial_carrier

		.globl _fuzixbios_lpt_busy
		.globl _fuzixbios_lpt_tx

		.globl _fuzixbios_disk_select
		.globl _fuzixbios_disk_set_lba
		.globl _fuzixbios_disk_read
		.globl _fuzixbios_disk_write
		.globl _fuzixbios_disk_flush
		.globl _fuzixbios_disk_param

		.globl _fuzixbios_rtc_get
		.globl _fuzixbios_rtc_set
		.globl _fuzixbios_rtc_secs

		.globl _callback_tty
		.globl _callback_timer
		.globl _callback_tick
		.globl _callback_kprintf

		.globl _do_callback_tty
		.globl _do_callback_timer

		.globl map_save_kernel
		.globl map_restore
		.globl interrupt_handler

		.globl _kprintf
;
;	Stubs
;
		.area _COMMONMEM
_callback_tty:
		ld (callback_sp),sp
		ld sp,#callback_stack
		call map_save_kernel
		call _do_callback_timer
		call map_restore
		ld sp,(callback_sp)
		ret
_callback_timer:
		ld (callback_sp),sp
		ld sp,#callback_stack
		call map_save_kernel
		call _do_callback_timer
		call map_restore
		ld sp,(callback_sp)
		ret
_callback_tick:
		jp interrupt_handler
_callback_kprintf:
		push hl
		call _kprintf
		pop hl
		ret

		.area _COMMONDATA

		.ds 64
callback_stack:
callback_sp:
		.dw 0

;
;	BIOS jump vectors
;
		.area _END

.end:

_fuzixbios_init			.equ	.end
_fuzixbios_reboot		.equ	.end+3
_fuzixbios_monitor		.equ	.end+6
_fuzixbios_getinfo		.equ	.end+9
_fuzixbios_set_callbacks	.equ	.end+12
_fuzixbios_param		.equ	.end+15
_fuzixbios_idle			.equ	.end+18
_fuzixbios_set_bank		.equ	.end+21

_fuzixbios_serial_txready	.equ	.end+24
_fuzixbios_serial_tx		.equ	.end+27
_fuzixbios_serial_setup		.equ	.end+30
_fuzixbios_serial_param		.equ	.end+33
_fuzixbios_serial_carrier	.equ	.end+36

_fuzixbios_lpt_busy		.equ	.end+39
_fuzixbios_lpt_tx		.equ	.end+42

_fuzixbios_disk_select		.equ	.end+45
_fuzixbios_disk_set_lba		.equ	.end+48
_fuzixbios_disk_read		.equ	.end+51
_fuzixbios_disk_write		.equ	.end+54
_fuzixbios_disk_flush		.equ	.end+57
_fuzixbios_disk_param		.equ	.end+60

_fuzixbios_rtc_get		.equ	.end+63
_fuzixbios_rtc_set		.equ	.end+66
_fuzixbios_rtc_secs		.equ	.end+69

_fuzixbios_init_done		.equ	.end+72
