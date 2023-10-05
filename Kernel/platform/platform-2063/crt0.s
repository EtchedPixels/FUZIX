        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area CODE-CODE2
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
	.area _VIDEO
        .area _CONST
	.area _SERIALDATA
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT      	; unused
        .area _GSFINAL     	; unused
        .area _BUFFERS     	; _BUFFERS grows to consume all before it (up to KERNTOP)
	; Discard is loaded where process memory wil blow it away
        .area _DISCARD
	.area _FONT
	; The rest grows upwards from C000 starting with the udata so we can
	; swap in one block, ending with the buffers so they can expand up
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
	; These get overwritten and don't matter
        .area _INITIALIZER	; binman copies this to the right place for us
        .area _COMMONMEM	; stuff we always need
	.area _COMMONDATA
	.area _SERIAL		; must be page aligned - we place it at
				; FE00-FFFF so it covers the loader space

	.area _PAGE0		; don't binpack

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
	.globl s__BUFFERS
	.globl l__BUFFERS
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler
	.globl outstring

	.include "kernel.def"

	; Starts at 0x1000 at the moment

	; Entered with bank = 0 from the bootstrap logic

	.area _CODE

	jp start
	.dw 0x10AE

start:
	ld sp, #kstack_top

	; move the common memory where it belongs    
;	ld hl, #s__DATA
;	ld de, #s__COMMONMEM
;	ld bc, #l__COMMONMEM
;	ldir
	; then the discard
	; Discard can just be linked in but is next to the buffers
;	ld de, #s__DISCARD
;	ld bc, #l__DISCARD
;	ldir

	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl),#0
	ldir

	; Zero buffers area
	ld hl, #s__BUFFERS
	ld de, #s__BUFFERS + 1
	ld bc, #l__BUFFERS - 1
	ld (hl), #0
	ldir

	call init_hardware
	call _fuzix_main
	; Should never return
	di
stop:	halt
	jr stop

	; Common starts page aligned so put vectors there
	.area	_COMMONMEM

	.globl	_vectors

_vectors:
	.ds	64
