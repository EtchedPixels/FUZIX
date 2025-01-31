/*
*	RAM disc helpers for CPC Standard Memory Expansions
*/

#include <kernel.h>

#undef di
#undef ei

#if defined EXTENDED_RAM_512 || defined EXTENDED_RAM_1024
static void CODESEG(void)  __naked { __asm .area _CODE __endasm; } /* .area _CODE */
void rd_io(void) __naked
{
	__asm

	.globl _rd_io

	.globl _block
	.globl _nblock
	.globl _rd_wr
	.globl _rd_swap_mem_port_h
	.globl _rd_swap_bank
	.globl _rd_proc_bank
	.globl _rd_swap_bank_addr
	.globl _rd_proc_bank_addr
	.globl _rd_dptr
	.globl _int_disabled
	.globl _vtborder
	.globl map_kernel_restore



	di
	ld a,(_rd_wr)
	or a
	jp z, is_wr
	ld	bc,#0x7f10
	out	(c),c
	ld	c,#0x57 ;Sky blue
	out	(c),c
	ld a,(_rd_swap_mem_port_h)
	ld b,a
	ld c,#0xff
	ld a,(_rd_swap_bank)
	out (c),a
	ld hl,(_rd_swap_bank_addr)
	ld de, #swapbuffer
	ld bc,#512
	ldir
	ld bc,#0x7fff
	ld a,(_rd_proc_bank)
	out (c),a
	ld hl,#swapbuffer
	ld de,(_rd_proc_bank_addr)
	ld bc,#512
	ldir
end_io:
	ld bc,#0x7fc1 ; map kernel
	out (c),c
	call map_kernel_restore ; do it the right way
	ld	bc,#0x7f10
	out	(c),c
	ld	a,(_vtborder)
	out	(c),a
	ld a,(_int_disabled)
	or a
	ret nz
	ei
	ret
is_wr:
	ld	bc,#0x7f10
	out	(c),c
	ld	c,#0x5C ;Red
	out	(c),c
	ld bc,#0x7fff
	ld a,(_rd_proc_bank)
	out (c),a
	ld hl,(_rd_proc_bank_addr)
	ld de,#swapbuffer
	ld bc,#512
	ldir
	ld a,(_rd_swap_mem_port_h)
	ld b,a
	ld c,#0xff
	ld a,(_rd_swap_bank)
	out (c),a
	ld hl,#swapbuffer
	ld de,(_rd_swap_bank_addr)
	ld bc,#512
	ldir
	jp end_io	

_rd_wr:
	.db 0
_rd_swap_bank:
	.db 0
_rd_proc_bank:
	.db 0
_rd_swap_mem_port_h:
	.db 0x7f	
_rd_swap_bank_addr:
	.dw 0
_rd_proc_bank_addr:
	.dw 0
_rd_dptr:
	.dw 0
_block:
	.dw 0
_nblock:
	.dw 0
swapbuffer:
	.ds 512

__endasm;
}
#endif

