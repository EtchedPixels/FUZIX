        .module crt0
	.z180

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
        .area _CONST
        .area _DISCARD
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
        .area _GSINIT
        .area _GSFINAL
        .area _COMMONMEM
        .area _INITIALIZER
	.area _PAGE0	   ; ROM don't pack

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

	.globl z80_irq

	.globl ___sdcc_enter_ix

	.include "kernel.def"
        .include "../cpu-z180/z180.def"

	.globl outchar

        ; startup code

        .area _CODE
;
;	ROM vectors
;
	di
	ld	sp,#0xFFFF
	jp	startup
; RST 8
	jp	___sdcc_enter_ix
	.ds	5
; RST 10
	ld	sp,ix
	pop	ix
	ret
	.ds	3
; RST 18
	pop	af
	pop	ix
	ret
	.ds	4
; RST 20
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
; RST 28
	ret
	.ds	7
; RST30
	ret
	.ds	7
; RST38
	jp	z80_irq
	.ds	5

;
;	The kernel lives in the low 64K of ROM.
;
startup:
	ld	a,#0xC0
	out0	(0x3F),a		; I/O at 0xC0
	ld	a,#0x64
	out0	(ASCI_CNTLA0),a
	xor	a
	out0	(ASCI_CNTLB0),a		; 38400 or 57600

	;	SRAM
	ld	a,#0x20			; 0 wait RAM 2 wait I/O
	out0	(DMA_DCNTL),a

	ld	a,#0x00
	out0	(MEM_RCR),a		; No refresh needed - SRAM
	
	;	Set up the windows

	ld	a,#0xFD
	out0	(MMU_CBAR),a
	ld	a,#0x42			; User bank 1
	out0	(MMU_CBR),a		; F000-FFFF User bank 1 common

	;	We are now in the kernel mapping but need to fish
	;	stuff out of the ROM for common etc

	ld	a,#0x01
	out0	(MMU_BBR),a		; E000 is now ROM F000
	ld	hl,#0xE000
	ld	de,#0xF000
	ld	bc,#0x1000
	ldir

	;
	;	Now initialize the RAM area from the ROM mapping.
	;

	ld	a,#0x33
	out0	(MMU_BBR),a		; Kernel data is now mapped
	ld	a,#0xFE
	out0	(MMU_CBR),a		; F000 is now ROM D000
	ld	hl,#0xF000
	ld	de,#0xD000
	ld	bc,#0x1000
	ldir
	inc	a
	out0	(MMU_CBR),a		; F000 is now ROM E000
	ld	hl,#0xF000
	ld	bc,#0x1000
	ldir

	; We've identity copied the D000-EFFF space from ROM

	; Set the common to be 51:000 to 51:FFF
	ld	a,#0x42
	out0	(MMU_CBR),a		; common back

	;	We now have a valid memory map, stack etc

        ld sp, #kstack_top

        ; Zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Configure memory map
        call init_early

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
