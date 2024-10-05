#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

        ; startup code

	.abs

	.org	0x100

init:                       ; must be at 0x0100 as we are loaded at that
	di

	ld	sp, kstack_top
	; Move the common memory where it belongs
	ld	hl, __data
	ld	de, __common
	ld	bc, __common_size
	ldir
;	ld	de, __commondata
;	ld	bc, __commondata_size
;	ldir
	; then the discard
	; Discard can just be linked in but is next to the buffers
	ld	de, __discard
	ld	bc, __discard_size
	ldir
	; then zero the data area
	ld	hl, __bss
	ld	de, __bss + 1
	ld	bc, __bss_size - 1
	ld	(hl), 0
	ldir
	; Zero buffers area
;	ld	hl, __buffers
;	ld	de, __buffers + 1
;	ld	bc, __buffers_size - 1
;	ld	(hl), 0
;	ldir

        ; Hardware setup
        call	init_hardware

        ; Call the C main routine
        call	_fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

	.buffers
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.export _bufpool

_bufpool:
	.ds BUFSIZE * NBUFS
