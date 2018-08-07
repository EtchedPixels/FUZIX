;
;	TODO
;

	.module floppy

	.area _HIGH

	.globl _fd_motor_on
	.globl _fd_operation
	.globl _fd_reset

_fd_operation:
_fd_reset:
_fd_motor_on:
	ret
