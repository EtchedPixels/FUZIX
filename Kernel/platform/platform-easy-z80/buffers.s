;
;	External buffer support logic
;

	.module buffers
	.area _COMMONMEM

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

	.globl _do_blkzero
	.globl _do_blkcopyk
	.globl _do_blkcopyul
	.globl _do_blkcopyuh

	.globl _bdest
	.globl _blen

	.globl map_kernel
	.globl map_buffers
	.globl map_buffers_user
	.globl map_buffers_user_h

	.globl _workbuf


;
;	Ugly - but we need to rework memory management and stuff to fix it
;
_workbuf:
	.ds 1024

_bdest:
	.word 0
_blen:
	.word 0

_do_blkzero:
	call map_buffers
	ld e,l
	ld d,h
	inc de
	ld (hl),#0
	ld bc,#511
	ldir
	jp map_kernel

_do_blkcopyk:
	call map_buffers
	ld de,(_bdest)
	ld bc,(_blen)
	ldir
	jp map_kernel

_do_blkcopyul:
	call map_buffers_user
	ld de,(_bdest)
	ld bc,(_blen)
	ldir
	jp map_kernel

_do_blkcopyuh:
	call map_buffers_user_h
	ld de,(_bdest)
	ld bc,(_blen)
	ldir
	jp map_kernel
