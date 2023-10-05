;
;	Support for the Atomlite IDE/CF interface
;
;	To support the Atom as well we need to make the 8bit selection in
;	devide_discard.c dynamic and we also have to address some corner
;	cases here because it's possible to get a word transfer across the
;	32K boundary when doing user space transfers. That means we need
; 	to be able to start half a word in. A transfer could also exceed
;	16K so clever alignment won't help us that much!
;
;	The other fun problem is that the atom expects correctly paired
;	transfers while atomlite just maps both data ports to the same
;	address. As we need to write latch,data and read data,latch we have
;	to worry about byteswapping.
;
        .module atom

	.include "kernel.def"
        .include "../kernel-z80.def"


	.globl _devide_read_data
	.globl _devide_write_data

	; imports
	.globl user_mapping
	.globl _blk_op
	.globl map_kernel_low


	.area _COMMONMEM

; FIXME: Move these somewhere better
BLKPARAM_ADDR_OFFSET		.equ	0
BLKPARAM_IS_USER_OFFSET		.equ	2
BLKPARAM_SWAP_PAGE		.equ	3

IDE_DATA_R	.equ 0x00F6
IDE_DATA_W	.equ 0x00F7

;
;	TODO: Use atom_ methods once written and we have them workable
;
_devide_read_data:
	ld a, (_blk_op + BLKPARAM_IS_USER_OFFSET)
	ld hl, (_blk_op + BLKPARAM_ADDR_OFFSET)
	or a
	jr z, atomlite_read
	dec a
	jr z, atomlite_read_user
; SWAP  to enable yet
;	ld a, (blk_op + BLKPARAM_SWAP_PAGE)
;	call map_for_swap
atomlite_read:
	call atomlite_reader_fast
	jp map_kernel_low
atomlite_read_user:
	push ix
	; HL is the user buffer, BC length
	ld ix,#_udata + U_DATA__U_PAGE
	call user_mapping
	jr z,ide_r2
	call atomlite_reader
	exx
	call user_mapping
ide_r2:
	call atomlite_reader
	pop ix
	jp map_kernel_low

_devide_write_data:
	ld a, (_blk_op + BLKPARAM_IS_USER_OFFSET)
	ld hl, (_blk_op + BLKPARAM_ADDR_OFFSET)
	or a
	jr z, atomlite_write
	dec a
	jr z, atomlite_write_user
; SWAP  to enable yet
;	ld a, (blk_op + BLKPARAM_SWAP_PAGE)
;	call map_for_swap
atomlite_write:
	call atomlite_writer_fast
	jp map_kernel_low
atomlite_write_user:
	push ix
	; HL is the user buffer, BC length
	ld ix,#_udata + U_DATA__U_PAGE
	call user_mapping
	jr z,ide_w2
	call atomlite_writer
	exx
	call user_mapping
ide_w2:
	call atomlite_writer
	pop ix
	jp map_kernel_low

;
;	Routines for the Atom IDE 16bit interface
;
;	Not yet finished
;
;
;	The non split case 512 bytes as fast as we can given the interface
;	design
;
atom_reader_fast:
	; Select data port
	ld a,#0x30
	out (0xF5),a
	ld bc,#0xF7	; 256 words, port F7
atom_rf_loop:
	ld a,(0xF6)
	ini		; Read from F6 for the data high
	ld (hl),a
	inc hl
	jr nz,atom_rf_loop
	ret
;
;	The non split case 512 bytes as fast as we can given the interface
;	design. For Atomlite we could just inir
;
atom_writer_fast:
	ld a,#0x30
	out (0xF5),a	; Select data port
	ld bc,#0xF7	; 256 words port F7
atomlite_wf_loop:
	ld a,(hl)
	inc hl
	outi
	out (0xF6),a
	jr nz, atomlite_wf_loop
	ret

;
;	The Atomlite is simpler
;

;
;	The Atomlite reader is simple
;
atomlite_reader_fast:
	ld a,#0x30
	out (0xF5),a
	ld bc,#0xF7
	inir
	inir
	ret

atomlite_writer_fast:
	ld a,#0x30
	out (0xF5),a
	ld bc,#0xF7	; 256 bytes port F7
	otir
	otir
	ret

atomlite_writer:
	ld a,#0x30
	out (0xF5),a	; Select data port
	ld d,b
	ld e,c
	ld bc,#0xF7
ide_w_loop:
	outi
	dec de
	ld a,d
	or e
	jr nz,ide_w_loop
	ret

.globl atomlite_reader	; for debugging work
atomlite_reader:
	ld a,#0x30
	out (0xF5),a	; Select data port
	ld d,b
	ld e,c
	ld bc,#0xF6
ide_r_loop:
	ini
	dec de
	ld a,d
	or e
	jr nz,ide_r_loop
	ret
