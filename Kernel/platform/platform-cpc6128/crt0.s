        .module crt0

	;
	;	High space - read only 
	;

        .area _CODE
	.area _CODE2
	;
	;	Try and keep code in the top 32K
	;


	;
	;	Our common lives low
	;
	.area _CODE3
        .area _VIDEO		; must end below 0x4000
		.area _FONT
		.area _INITIALIZED
        .area _HOME
	.area _CONST

	;
	;	Beyond this point we just zero.
	;

        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL
	
	;
	;	Finally the buffers so they can expand
	;
	.area _BUFFERS
	; Somewhere to throw it out of the way
        .area _INITIALIZER

        .area _DISCARD


        .area _COMMONMEM

		

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
	.globl l__BUFFERS
	.globl s__BUFFERS
	.globl l__COMMONMEM
	.globl s__COMMONMEM
	.globl l__DATA
	.globl s__DATA
	.globl l__DISCARD
	.globl s__DISCARD
	.globl l__FONT
	.globl s__FONT
        .globl kstack_top

        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

	;
        ; startup code
	;
	; We loaded the rest of the kernel from disk and jumped here
	;

        .area _CODE

	.globl _start



_start:


	di
        ld sp, #kstack_top
	;
	; move the common memory where it belongs    
	ld hl, #s__DATA
	ld de, #s__COMMONMEM
	ld bc, #l__COMMONMEM
	ldir

	; then the font
;	ld de, #s__FONT
;	ld bc, #l__FONT
;	ldir

	; then the discard (backwards as will overlap)
	ld de, #s__DISCARD
	ld bc, #l__DISCARD-1
	ex de,hl
	add hl,bc
	ex de,hl
	add hl,bc
	lddr
	ldd
	
	; then zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	ldir
	; and buffers
	ld hl, #s__BUFFERS
	ld de, #s__BUFFERS + 1
	ld bc, #l__BUFFERS - 1
	ld (hl), #0
	ldir

		;We are loading from a .sna with first 64k filled with fuzix.bin starting at 0x100
	;copy bank 3 to bank 7 in C7 mode (7 at 0x4000), zero bank 3 (vmem) and switch to C1 (kernel map)
	;when a proper loader is done this should be managed there
	
		ld bc,#0x7fc7
		out (c),c

		ld hl, #0xc000
		ld de, #0x4000
		ld bc, #0x4000
		ldir
		
		ld bc,#0x7f00
		out (c),c
		ld a,#0x44
		out (c),a		; blue paper
		ld bc,#0x7f01
		out (c),c
		ld a,#0x4b
		out (c),a		; white ink

		ld hl, #0xc000
		ld de, #0xc001
		ld bc, #0x3fff
		ld (hl), #0
		ldir

		ld bc,#0x7fc1
		out (c),c

		ld hl,#copyfont
		ld de,#0xF000
		ld bc,#(copyfont_end-copyfont)
		ldir

		call #0xF000

		ld hl,#0xF000
		ld de,#0xF001
		ld bc,#(copyfont_end-copyfont-1)
		ld (hl),#0
		ldir

        ; Configure memory map
        call init_early

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

		;code to copy font
copyfont: ;;this wil be in the loader
		di
        ld bc, #0x7faa ;RMR ->UROM disable LROM enable
        out (c),c
        ld hl, #0x3800 ;Firmware (LROM) character bitmaps
		ld de, #(_fontdata_8x8) 
		ld bc, #0x800
		ldir
		ld bc, #0x7fae ;RMR ->UROM disable LROM disable
        out (c),c
		ret		
copyfont_end:


	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS

	.globl _fontdata_8x8
	.area _FONT
	_fontdata_8x8:
	.ds 2048
