
        .module crt0

	; We use a fairly normal ordering as we've not got any horrible
	; tricks to pull with our fairly linear memory

        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
        .area _INITIALIZER ; binman copies this to the right place for us
	; These get overwritten and don't matter
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
	;	Common space starts at F000 for now
        .area _COMMONMEM

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler

	.include "kernel.def"

	; Starts at 0x0100
	;
	; We came here from CP/M (for now anyway)
	; That means our MMU setup is that we are in bank 1 with an 8K
	; common. The MMU preloads are set to 3F, 7F, 3E, 7E
	; D507 is 00:00 D5808 is 01:00 (so low is mapped to bank 0 and 1:1)
	; RAM config is set to 0B and MMU config to B0

	.area _BOOT

init:  
	di
	; 	Write a di/halt just before the CPU switching to help debug
	;	any wild adventures wandering into this code before they happen
	ld	hl,#0x76F3
	ld	(0xFFCE),hl
	;	Select bank 1 I/O on
	ld	a,#0x7E		; bank 1 I/O on
	ld	(0xFF00),a
	;	Now set up the LCR
	ld	a,#0x3F
	ld	(0xD501),a	; PCR A		bank 0
	ld	a,#0x7F
	ld	(0xD502),a	; PCR B		bank 1
	ld	a,#0x3E
	ld	(0xD503),a	; PCR C		bank 0 with I/O
	ld	a,#0x7E
	ld	(0xD504),a	; PCR D		bank 1 with I/O
	;	D505 should be correct (Z80, C128 mode)
	ld	a,#0x0A
	ld	(0xD506),a	; 8K common, top is common
	;	Page ptrs should be fine : TODO review

	ld	(0xFF02),a	; bank 1 no I/O
	; Move the copier routine into common space
	ld	hl,#copier
	ld	de,#0xFE00
	ld	bc,#0x0080
	ldir
	; and run it
	jp	0xFE00
copier:
	ld	hl,#0x1400
	ld	bc,#0xCC00		; copy 1400-DFFF
initcp:
	ld	a,(hl)
	ld	(0xFF01),a		; bank 0
	ld	(hl),a
	ld	(0xFF02),a		; bank 1
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz, initcp

	ld	(0xFF03),a		; bank 0, I/O on

	ld	a,#0x46
	ld	bc,#0xD018
	out	(c),a

	; Now begin setting up the video as the most important thing
	; is being able to see wtf is going on

	;	CIA2A bits 1-0 are A15 and A14 for the 14 bit VIC
	ld	bc,#0xDD00
	in	a,(c)
	or	#0x03			; low 16K for the video
	out	(c),a

	ld	(0xFF01),a		; bank 0, I/O off

	jp	go

	.area	_CODE
go:
	;	The copier returns here with the code in both banks
	;	FCxx is FDFDFDFDFD up to FD01, FDFD is then the vector
	ld	a,#0xfc
	ld	i,a
	im	2
	;	TODO - vic raster int ?

        ; switch to stack in common memory
        ld sp, #kstack_top


	;	Move things into place
	ld	hl, #s__DATA
	ld	de, #s__COMMONMEM
	ld	bc, #l__COMMONMEM
	ldir

	;	discard - in code for now
	ld	de, #s__DISCARD
	ld	bc, #l__DISCARD-1
	add	hl,bc
	ex	de,hl
	add	hl,bc
	ex	de,hl
	inc	bc
	lddr

	;	clear bss
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl),#0
	ldir

	;	clear video
	ld	hl,#0x1000
	ld	de,#0x1001
	ld	bc,#999
	ld	(hl),#' '
	ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
