;
;	Virtual disks
;

		.module vd

		.globl _vd_do_open
		.globl _vd_do_op

		.globl _vd_track
		.globl _vd_sector
		.globl _vd_dpb
		.globl _vd_drive_op

		.globl map_for_swap
		.globl map_proc_always
		.globl map_buffers
		.globl map_kernel

		.globl _td_raw
		.globl _td_page

		.area _COMMONMEM

_vd_do_open:
		push ix
		push hl
		pop ix
		ld b,(hl)
		ld a,#3
		.dw 0xfeed
		ld l,a
		pop ix
		ret

_vd_do_op:
		push ix
		push iy
		push hl
		pop iy
		; Load all the registers before we go
		ld ix,(_vd_dpb)
		ld hl,(_vd_track)
		ld de,(_vd_sector)
		ld bc,(_vd_drive_op)	; B drive C op
		ld a,(_td_raw)
		cp #2
		jr nz, not_swap
		ld a,(_td_page)
		call map_for_swap
		jr do_op
not_swap:
		or a
		jr z, op_kernel
		call map_proc_always
		jr do_op
op_kernel:
		call map_buffers
do_op:
		; Beware the trap updates IY as well as A
		ld a,c
		.dw 0xfeed
		ld l,a
		pop iy
		pop ix
		jp map_kernel
