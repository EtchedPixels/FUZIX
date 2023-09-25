	.globl _devide_read_data
	.globl _devide_write_data

	.globl map_proc_always
	.globl map_proc_a
	.globl map_kernel_restore

	.globl _td_raw
	.globl _td_page

	.module ide
;
;	If we ever do a native driver for M3SE we'll want to do something
;	different here for that and use ide_select to switch interface types
;

IDE_REG_DATA			.equ	0x40

	.area _COMMONMEM

map:
	ld	a,(_td_raw)
	ld	bc,#IDE_REG_DATA
	; Swap not yet supported
	or	a
	ret	z	; Kernel - no remap
	dec	a
	jp	z, map_proc_always
	ld	a,(_td_page)
	jp	map_proc_a

_devide_read_data:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	map
	inir
	inir
unmap:
	ld	a,(_td_raw)
	or	a
	ret	z
	jp	map_kernel_restore

_devide_write_data:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	map
	otir
	otir
	jr	unmap
