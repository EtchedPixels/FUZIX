;
;	Gemini 833 Ramdisc support code
;
	.module gm833

	.include "kernel.def"
	.include "../kernel-z80.def"

	.globl _gm833_in
	.globl _gm833_out

	.globl map_process_a
	.globl map_kernel
	.globl _io_page

	.area _COMMONMEM

_gm833_in:
	ld a,(_io_page)
	or a
	call nz, map_process_a
	ld bc,#0x80FD
	inir
	jp map_kernel
_gm833_out:
	ld a,(_io_page)
	or a
	call nz, map_process_a
	ld bc,#0x80FD
	otir
	jp map_kernel
