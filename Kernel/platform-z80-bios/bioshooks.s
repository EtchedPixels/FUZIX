
		.module bioshooks

		.globl _fuzixbios_reboot
		.globl _fuzixbios_monitor
		.globl _fuzixbios_getinfo
		.globl _fuzixbios_set_callbacks
		.globl _fuzixbios_param
		.globl _fuzixbios_idle

		.area _END

.end:

_fuzixbios_init			.equ	.end
_fuzixbios_reboot		.equ	.end+3
_fuzixbios_monitor		.equ	.end+6
_fuzixbios_getinfo		.equ	.end+9
_fuzixbios_set_callbacks	.equ	.end+12
_fuzixbios_param		.equ	.end+15
_fuzixbios_idle			.equ	.end+18
_fuzixbios_serial_txready	.equ	.end+21
_fuzixbios_serial_tx		.equ	.end+24
_fuzixbios_serial_setup		.equ	.end+27
_fuzixbios_serial_param		.equ	.end+30
_fuzixbios_lpt_busy		.equ	.end+33
_fuzixbios_lpt_tx		.equ	.end+36

_fuzixbios_disk_select		.equ	.end+39
_fuzixbios_disk_set_lba		.equ	.end+42
_fuzixbios_disk_read		.equ	.end+45
_fuzixbios_disk_write		.equ	.end+48
_fuzixbios_disk_flush		.equ	.end+51
_fuzixbios_disk_param		.equ	.end+54


