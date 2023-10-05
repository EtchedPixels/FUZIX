;
;	Z80 DMA helpers
;
;	Initial memory copier helper and probe
;
;	TODO: burst I/O helpers to see if we can use them with the CF driver
;

		.module z80dma

		.include "../build/kernel.def"
		.include "../../cpu-z80/kernel-z80.def"

		.globl ldir_or_dma
		.globl sector_dma_in
		.globl sector_dma_out
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
				; (continuous, 16bit addr follows. no int)
dma_memdst:	.dw 0		; Address
		.db 0x92;82?	; Stop on end, /ce & /wait, ready active low
		.db 0xCF	; Load
		.db 0xB3	; Force ready (ignore /ce and /wait)
		.db 0x87	; Enable DMA


;	FIXME: memio has special rules we need to add
;	Write B fixed dest port in W4
;	Set WR0 bit 2 = 0 (B is source)
;	Load command
;	Write port A to WR0
;	Declare port A as source WR0 bit 2 = 1
;	Load port A with Load
;
;
script_memio:
		.db 0xC3	; Reset
		.db 0x79	; Set Port A address and length 16bit
mio_memsrc:	.dw 0		; transfer mode from B->A
mio_memlen:	.dw 0
		.db 0x14	; Set port A memory, incrementing
		.db 0x38	; Set port B fixed, I/O (28?)
		.db 0x80	; C0enables DMA No matching
		.db 0xAD	; Port B timing and interrupt config
				; (continuous, 16bit addr follows. no int)
mio_memdst:	.dw 0		; Address
		.db 0x92;82?	; Stop on end, /ce & /wait, ready active low
		.db 0xCF	; Load register B, reset counter
		.db 0x05	; Change direction A->B
		.db 0xCF	; Load register A as port address, set counter
		.db 0xB3	; Force ready (ignore /ce and /wait)
		.db 0x87	; Enable DMA

;
;	IDE disk read and similar. Continuous DMA rather than handshaking as
;	with a floppy controller
;
script_iomem:
		.db 0xC3	; Reset
		.db 0x7D	; Set Port A address and length 16bit
iom_memsrc:	.dw 0		; transfer mode from A->B
iom_memlen:	.dw 0
		.db 0x3C	; Set port A I/O fixed
		.db 0x10	; Set port B increments, is memory
		.db 0x80	; C0enables DMA No matching
		.db 0xAD	; Port B timing and interrupt config
				; (continuous, 16bit addr follows. no int)
iom_memdst:	.dw 0		; Address
		.db 0x92;82?	; Stop on end, /ce & /wait, ready active low
		.db 0xCF	; Load
		.db 0xB3	; Force ready (ignore /ce and /wait)
		.db 0x87	; Enable DMA

ldir_or_dma:
		ld a,b
		or a
		jr nz, dma_memcpy
		ld a,c
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

;
;	Load from CF or similar device flat out with no DMA handshaking
;
sector_dma_in:
		ld (iom_memsrc),bc		; port (high will be 0)
		ld (iom_memdst),hl		; memory
		ld bc,#511
		ld (iom_memlen),bc		; 512 bytes
		ld hl,#script_iomem
		ld bc,#(16*256 + DMAPORT)
		otir
		; CPU stalls until DMA done
		ld hl,(iom_memdst)
		; Return with expected HL
		inc h
		inc h
		ret

;
;	Write to CF or similar device flat out with no DMA handshaking.
;
sector_dma_out:
		ld (mio_memsrc),hl		; source address
		ld (mio_memdst),bc		; I/O (B = 0)
		ld bc,#511
		ld (mio_memlen),bc		; 512 bytes
		ld hl,#script_memio
		ld bc,#(18*256 + DMAPORT)
		otir
		; CPU stalls until DMA done
		ld hl,(mio_memdst)
		; Return with expected HL
		inc h
		inc h
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
		ld hl,#0xB0ED		; set up for ldir patch
		ld (ldir_or_dma),hl	; ldir
		ld a,#0xC9
		ld (ldir_or_dma + 2),a	; ret
		ld hl,#0xB2ED		; set up for inir patch
		ld (sector_dma_in),hl	; inir
		ld (sector_dma_in+2),hl	; inir
		ld (sector_dma_in+4),a	; ret
		inc h			; turns it into otir
		ld (sector_dma_out),hl	; otir
		ld (sector_dma_out+2),hl; otir
		ld (sector_dma_out+4),a	; ret		
		ld hl,#0xFFFF
		ret
dmatag:
		.dw 0xA5C0
dmatest:
		.dw 0xFFFF
