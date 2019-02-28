#include "../kernel-8080.def"
!
!	I/O mapped ram drive interface
!
	.sect .text

	.define _rd_present

_rd_present:
	ldsi 2
	lhlx			! L = board to probe for
	lxi d,0
	mov a,l
	rlc			! Board into bits 3-5 (board select)
	rlc
	rlc
	mov l,a
	out 0xC7
	xra a
	out 0xC7
	out 0xC7		! We should now be pointed to the first byte
	mvi a,0xAA
	out 0xC6		! Write AA55
	mvi a,0x55
	out 0xC6
	mov a,l
	out 0xC7
	xra a
	out 0xC7
	out 0xC7
	in 0xC6
	cpi 0xAA
	rnz
	in 0xC6
	cpi 0x55
	rnz
	inx d			! 1 = found
	ret

	.sect .common

	.define _rd_input
	.define _rd_output

rd_setup:
	ldsi 6			! Second argument (block number), allowing
				! for extra call
	lhlx
	mov a,h
	out 0xC7
	mov a,l
	out 0xC7
	xra a
	out 0xC7
	ldsi 8			! Third argument (user page)
	lhlx
	mov a,l
	mvi d,0			! 256 bytes to do
	ldsi 4			! First argumnent (target0, allowing for call)
	lhlx			! into HL

	ora a			! Kernel mapped
	rz
	jmp map_process_a	! User mapped or swap

_rd_input:
	call rd_setup
rd_inloop:
	in 0xC6
	mov m,a
	inx h
	dcr d
	jnz rd_inloop
	jmp map_kernel

_rd_output:
	call rd_setup
rd_outloop:
	mov a,m
	out 0xC6
	inx h
	dcr d
	jnz rd_outloop
	jmp map_kernel
