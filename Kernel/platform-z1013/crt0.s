        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area from C000
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
	; Discard is loaded where process memory wil blow it away
        .area _DISCARD
        .area _INITIALIZER ; binman copies this to the right place for us
	; Writeable memory segments
        .area _COMMONMEM
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
	.area _PAGE0	   ; force the bin packer to leave it alone

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
	.globl s__VECTORS
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler

	.globl pio0_intr

	.include "kernel.def"

	; Starts at 0x5FF0

	.area _VECTORS

	; Spot for interrupt vectors
	.word pio0_intr			; pio A
	.word interrupt_handler		; ctc channel 1

	; Starts at 0x8000

	.area _CODE

init:  
        di
	ld sp,#0x8000		; safe spot

	in a,(4)		; turn on 4MHz and work around bug? in JKCEMU
	or a,#0x60		; (wrong in a,(4) if boot with EPROM mapped)
	out (4),a

	im 2
	ld a,#(s__VECTORS >> 8)
	ld i,a

	; Clear the screen
	ld hl,#0xEC00
	ld de,#0xEC01
	ld bc,#0x03FF
	ld (hl),#' '
	ldir
	ld ix,#0xEC00

	; Load and map the rest of the image
	ld d,#3			; FIXME
	ld bc,#0x48
	ld hl,#0x0100		; Load 0100 upwards
	xor a

	call init_gide
loader:
	ld (ix),#'='
	inc ix
	call load_sector
	inc d
	bit 6,d			; load 2 - 63
	jr z, loader

        ; switch to stack in common memory
        ld sp, #kstack_top

        ; Zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

	in a,(4)		; if possible map out the system ROM
	or #0x10		; and video memory
	out (4),a

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

;
;	Load sector d from disk into HL and advance HL accordingly
;
load_sector:
	ld a,d
	out (0x4B),a		; LBA / sector
	ld a,#1
	out (0x4A),a		; 1 sector
	ld a,#0x20
	out (0x4F),a		; command
	; Wait 
wait_drq:
	in a,(0x4F)
	bit 3,a
	jr z, wait_drq
	; Get data, leave HL pointing to next byte, leaves B as 0 again
	inir
	inir
	ret
wait_ready:
	in a,(0x4F)
	bit 6,a
	jr z,wait_ready
	ret

init_gide:
	ld a,#0xE0
	out (0x4E),a
	xor a
	out (0x4C),a
	out (0x4D),a
	ret
