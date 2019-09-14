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
		.db 0xC3	; Reset
		.db 0x7D	; Set Port A address and length 16bit
dma_memsrc:	.dw 0		; transfer mode from A->B
dma_memlen:	.dw 0
		.db 0x14	; Set port A memory, incrementing
		.db 0x10	; Set port B increments, is memory
		.db 0x80	; C0enables DMA No matching
		.db 0xAD	; Port B timing and interrupt config
				; (burst, 16bit addr follows. no int)
dma_memdst:	.dw 0		; Address
		.db 0x92;82?	; Stop on end, /ce & /wait, ready active low
		.db 0xCF	; Load
		.db 0xB3	; Force ready (ignore /ce and /wait)
		.db 0x87	; Enable DMA


ldir_or_dma:
		ld a,h
		or a
		jr nz, dma_memcpy
		ld a,l
		cp #32
		jr nc, dma_memcpy
		ldir
		ret
dma_memcpy:
		ld (dma_memsrc),hl
		ld (dma_memdst),de
		; The DMA transfers one more than the specified count
		; in sequential transfer mode (Table 11)
		dec bc
		ld (dma_memlen),bc
		ld hl,#script_memmem
		ld bc,#(16*256 + DMAPORT)
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
