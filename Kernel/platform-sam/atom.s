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
        .include "../kernel.def"


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
	call atomlite_reader
	jp map_kernel_low
atomlite_read_user:
	; HL is the user buffer, BC length
	call user_mapping
	jr z,ide_r2
	call atomlite_reader
	exx
	call user_mapping
ide_r2:
	call atomlite_reader	
	jp map_kernel_low

_devide_write_data:
	ld a, (_blk_op + BLKPARAM_IS_USER_OFFSET)
	ld hl, (_blk_op + BLKPARAM_ADDR_OFFSET)
	or a
	jr z, atomlite_read
	dec a
	jr z, atomlite_read_user
; SWAP  to enable yet
;	ld a, (blk_op + BLKPARAM_SWAP_PAGE)
;	call map_for_swap
atomlite_write:
	call atomlite_writer
	jp map_kernel_low
atomlite_write_user:
	; HL is the user buffer, BC length
	call user_mapping
	jr z,ide_w2
	call atomlite_writer
	exx
	call user_mapping
ide_w2:
	call atomlite_writer
	jp map_kernel_low
;
;	This needs optimizing to use as we know C = 0 - but think about
;	the hard case with atom and split transfers. Probably need to
;	unroll the loop into two halves, check termination on each and also
;	somehow indicate partial transfers. We do know the transfer will be
;	a total of 512 bytes so the rule I think is
;	if bit 0,c on 1st transfer - it's split word 
;
atomlite_reader:
	ld d,b
	ld e,c
	ld bc,#IDE_DATA_R
ide_r_loop:
	in a,(c)
	ld (hl),a
	inc hl
	dec bc
	ld a,b
	or c
	jr nz,ide_r_loop
	ret
;
;	The non split case 512 bytes as fast as we can given the interface
;	design
;
atomlite_reader_fast:
	ld bc,#IDE_DATA_R
atomlite_rf_loop:
	ini		; Read from F6 for the data high
	inc b
	inc b		; up to F7 for the data low
	ini
	dec a
	jr nz, atomlite_rf_loop
	ret
;
;	This needs optimizing to use as we know C = 0
;
atomlite_writer:
	ld d,b
	ld e,c
	ld bc,#IDE_DATA_W
ide_w_loop:
	ld a,(hl)
	out (c),a
	inc hl	
	dec bc
	ld a,b
	or c
	jr nz,ide_w_loop
	ret
;
;	The non split case 512 bytes as fast as we can given the interface
;	design
;
atomlite_writer_fast:
	ld bc,#IDE_DATA_W
atomlite_wf_loop:
	ini		; Read from F7 for the data high
	ini		; F6 for the data low
	inc b
	inc b		; back to F7
	dec a
	jr nz, atomlite_rf_loop
	ret
