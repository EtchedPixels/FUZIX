;
;	RAM disc helpers for DivIDE Plus
;
;	We will make even COMMONMEM vanish whilst we work!
;

	.area _COMMONDATA

	.globl _rd_io

	.globl _rd_wr
	.globl _rd_dptr
	.globl _rd_page
	.globl _rd_addr


	.globl _int_disabled
	.globl map_bank_a
	.globl map_kernel_restore


_rd_io:
	di
	ld a,(_rd_page)
	call map_bank_a
	ld hl,(_rd_addr)
	ld de,(_rd_dptr)
	ld bc,#512
	ld a,(_rd_wr)
	or a
	jr z, is_wr
do_io:
	ldir
	call map_kernel_restore
	ld a,(_int_disabled)
	or a
	ret nz
	ei
	ret
is_wr:
	ex de,hl
	jr do_io


_rd_wr:
	.db 0
_rd_dptr:
	.dw 0
_rd_page:
	.db 0
_rd_addr:
	.dw 0
