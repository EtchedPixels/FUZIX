
	.abs

	.org	0x1000
	jp	start


        .code

start:
	;	I/O is at 0x0000-0x0FFF, kernel starts at 0x1000 all is good
	di
	; Zero the data area
	ld	hl, __bss
	ld	de, __bss + 1
	ld	bc, __bss_size - 1
	ld	(hl), 0
	ldir

;		Zero buffers area
	ld	hl, __buffers
	ld	de, __buffers+1
	ld	bc, __buffers_size - 1
	ld	(hl), 0
	ldir

	ld	sp,kstack_top
	call	init_early
	call	init_hardware
	call	_fuzix_main
	di
stop:	halt
	jr	stop

	