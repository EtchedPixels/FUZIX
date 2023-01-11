
        .module crt0

	; We use a fairly normal ordering as we've not got any horrible
	; tricks to pull with our fairly linear memory

	; Start with the ROM area from C000
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
	; These get overwritten and don't matter
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
        .area _INITIALIZER ; binman copies this to the right place for us
	.area _FONT	   ; Font buffer
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
        .globl s__FONT
        .globl l__FONT
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler

	.include "kernel.def"

	; Starts at 0x0100

	.area _CODE

init:  
        di
        ; switch to stack in common memory
        ld sp, #kstack_top

	ld	a,#0x7F
	ld	(0xFF00),a
	;	VIC bank 1, common top, size 4K
	ld	a,#0x49
	;	test hack (common 8K top E000-FFFF)
	ld	a,#0x0A
	ld	bc,#0xD506
	out	(c),a
	; Now begin setting up the video as the most important thing
	; is being able to see wtf is going on

	ld	bc,#0xD018
	ld	a,#0x96	;A		; TODO use ROM font and remap ?
	;	VIC font at E800, bank 1
	;	VIC screen at E400, bank 1
	out	(c),a
	;	CIA2A bits 1-0 are A15 and A14 for the 14 bit VIC
	ld	bc,#0xDD00
	in	a,(c)
	or	#0x03		; go top 16K
	out	(c),a

	;	Move things into place
	ld	hl, #s__DATA
	ld	de, #s__COMMONMEM
	ld	bc, #l__COMMONMEM
	ldir
	;	font
	ld	de, #s__FONT
	ld	bc, #l__FONT
	ldir
	;	discard
	ld	de, #s__DISCARD
	ld	bc, #l__DISCARD
	ldir
	;	clear bss
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl),#0
	ldir

	;	clear video
	ld	hl,#0xE400
	ld	de,#0xE401
	ld	bc,#999
	ld	(hl),#'Z'
	ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
