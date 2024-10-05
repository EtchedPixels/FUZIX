	.globl _devide_read_data
	.globl _devide_write_data

	.globl map_process_always
	.globl map_kernel_restore

	.globl _blk_op

	.module ide
;
;	If we ever do a native driver for M3SE we'll want to d something
;	different here for that and use ide_select to switch interface types
;
; FIXME: Move these somewhere better
BLKPARAM_ADDR_OFFSET		.equ	0
BLKPARAM_IS_USER_OFFSET		.equ	2
BLKPARAM_SWAP_PAGE		.equ	3

IDE_REG_DATA			.equ	0x40

	.area _COMMONMEM

_devide_read_data:
	ld a,(_blk_op + BLKPARAM_IS_USER_OFFSET)
	ld hl,(_blk_op + BLKPARAM_ADDR_OFFSET)
	ld bc,#IDE_REG_DATA
	; Swap not yet supported
	or a
	jr z, no_map
	push af
	call map_process_always
	pop af
no_map:
	inir
	inir
	or a
	ret z
	push af
	call map_kernel_restore
	pop af
	ret

_devide_write_data:
	ld a,(_blk_op + BLKPARAM_IS_USER_OFFSET)
	ld hl,(_blk_op + BLKPARAM_ADDR_OFFSET)
	ld bc,#IDE_REG_DATA
	; Swap not yet supported
	or a
	jr z, no_mapw
	push af
	call map_process_always
	pop af
no_mapw:
	otir
	otir
	or a
	ret z
	push af
	call map_kernel_restore
	pop af
	ret
