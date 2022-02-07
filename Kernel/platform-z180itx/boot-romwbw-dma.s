;
;	ROMWBW boot block
;
;	The provided bootloader is a bit limited. It can only really load
;	stuff into upper memory and assumes CP/M is being loaded
;
;	Unfortunately this means we have to chain our own loader.
;	Fortunately ROMWBW is really quite nice so it's easy to use to do
;	the load. What it can't do however is do I/O into the low 32K
;	directly - it has no separate I/O target page feature it seems.
;
;	We build the lot as if it's a binary at 0xF000 as that's easier than
;	fighting the linker and the like
;
;	The end must actually be the byte after we need. ROMWBW explodes
;	if given F200 F3FF !
;
		.module boot

		.z180

	.include "kernel.def"
	.include "../cpu-z180/z180.def"

.area	    BOOT	(ABS)
	.org	    0xF000

	.ds   384
	.byte 0x5A
	.byte 0xA5
	.byte 0x00	; formatting platform
	.byte 0x00	; device
	.byte 0x00	; formatter
	.byte 0x00	; physical drive
	.byte 0x00	; logical unit
	.byte 0x00	; unused
	.ds 88		; move to byte 96
	.byte 0x00	; write protect
	.word 0x00	; update count
	.byte 0		; major
	.byte 0		; minor
	.byte 0		; update
	.byte 0		; patch
	.ascii 'Fuzix 111 Loader'
	.byte '$'	; indicate valid label
	.word 0		; no patch
	.word 0xF200	; load target
	.word 0xF400	; end - way more than we need but that's fine
	.word 0xF200	; run from here

;
;	This should be 0xF200. We are entered with the stack somewhere
;	in the middle of RAM, HBIOS stubs in the top 2 pages and RAM
;	low including our expected RST hooks for HBIOS
;
;	We need to go get our device/subunit back from the BIOS because
;	it's has RST 8 interfaces for this rather than just passing
;	them in a register.
;
bootit:
	ld sp, #0xFE00		; SP as high as we can

	call light

	ld a,#0x02
	out0 (DMA_DMODE),a	; memory to memory burst mode

	ld bc, #0xF8F0		; Get the CPU info into H L and speed into
				; DE
	rst 8

	push hl
	push de

	ld bc, #0xF100		; Get system type into L
	rst 8

	push hl

	ld bc, #0xF8E0		; Get boot sysinfo into DE
	rst 8

	call light

	ld b, #0x13		; ROMWBW disk read request
	ld c, d			; Device (same as booted off)

	call load_16k
	call load_16k
	call load_16k
	call load_16k

	;
	;	Map the kernel low 16K block and jump into it
	;
	di
	ld a,#0x80		; First RAM bank
	out (MMU_BBR),a		; And go
	ld a,#0xff
	out (0x0D),a
	jp 0x0100

light:
	ld a,(lightval)
	out (0x0D),a
	inc a
	ld (lightval),a
	ret

load_16k:
	call light
	ld hl, #0x8000		; Loading at 0x8000 for the moment
	ld e, #32		; 32 sectors (16K)
	push bc
	rst 8			; Can error but if so wtf do we do ?

	call light
	call dma_bank
	call light

	pop bc
	ret
	
dma_bank:
	; Copy 16K into the next block
	ld bc,#0x4000
	out0 (DMA_BCR0H),b		; 16K to copy
	out0 (DMA_BCR0L),c

	ld h,c
	in0 l,(MMU_CBR)			; and from the current CPU bank
	add hl,hl			; Can't assume 64K aligned..
	add hl,hl			; Turn H into bank
	add hl,hl			; and L into offset in 256 byte
	add hl,hl			; pages. C is already 0
	ld de,#0x80
	add hl,de			; move on 32K
	out0 (DMA_SAR0B),h		; 64K bank
	out0 (DMA_SAR0H),l		; From 0x8000
	out0 (DMA_SAR0L),c		;

	ld de,(dma_target)		; target in e, page in d
	out0 (DMA_DAR0B),d		; 0x08 - first RAM bank
	out0 (DMA_DAR0H),e		; 16K chunk
	out0 (DMA_DAR0L),c		; Always 0 as aligned
	out0 (DMA_DSTAT),b		; 0x40 (enable DMA)
	; And CPU will continue once the DMA is done

	ld a,b				; b is 0x40
	add e				; Move on 0x4000 bytes
	ld (dma_target),a
	ret nz
	inc d
	ld a,d
	ld (dma_page),a
	ret

dma_target:
	.byte 0x00
dma_page:
	.byte 0x08
lightval:
	.byte 0x01
