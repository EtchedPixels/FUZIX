;
;	Z80 DMA helpers
;
;	Initial memory copier helper and probe
;
;	TODO: burst I/O helpers to see if we can use them with the CF driver
;

		.module z80dma

		.include "../platform/kernel.def"
		.include "../kernel-z80.def"

		.globl ldir_or_dma
		.globl dma_memcpy
		.globl _probe_z80dma

		.area _COMMONMEM

script_memmem:
		.db 0xC3
		.db 0xC7
		.db 0XCB
		.db 0X7D
dma_memsrc:	.dw 0
dma_memlen:	.dw 0
		.db 0x14
		.db 0x10
		.db 0xC0
		.db 0xAD
dma_memdst:	.dw 0
		.db 0x92
		.db 0xCF
		.db 0xB3
		.db 0x87


ldir_or_dma:
		ld a,h
		or a
		jr nz, dma_memcpy
		ld a,l
		cp #32
		jr z, dma_memcpy
		ldir
		ret
dma_memcpy:
		ld (dma_memsrc),hl
		ld (dma_memdst),de
		ld (dma_memlen),bc
		ld hl,#script_memmem
		ld bc,#(18 + DMAPORT)
		otir
		; CPU stalls until DMA done
		ret


		.area _DISCARD
;
;	Tell the DMA controller to DMA a word and see if anything happens
;
;	If it is absent then fastpath the ldir_or_dma call into an LDIR RET
;
_probe_z80dma:
		ld hl,#dmatag
		ld de,#dmatest
		ld bc,#2
		call dma_memcpy
		ld hl,(dmatest)
		ld de,#0xA5C0
		or a
		sbc hl,de
		ret z
		ld hl,#0xB0ED
		ld (ldir_or_dma),hl
		ld a,#0xC9
		ld (ldir_or_dma + 2),a
		ld hl,#0xFFFF
		ret
dmatag:
		.dw 0xA5C0
dmatest:
		.dw 0xFFFF
