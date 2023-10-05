	; Code block (can be top of banked space or in motherboard memory)
        .area _CODE
        .area _CODE2
	.area _HOME
	.area _VIDEO
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _GSINIT
        .area _GSFINAL
	; Buffers must be directly before discard as they will
	; expand over it
	.area _BUFFERS
	.area _DISCARD
        .area _COMMONMEM

        .area _COMMONDATA
        ; Doesn't matter if these go over the I/O space as they are
	; removed at the end of the build
        .area _INITIALIZER

	; tell the packer to leave us alone
	.area _PAGE0

	; imported symbols
	.globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__DATA
        .globl l__DATA
	.globl s__DISCARD
	.globl l__DISCARD
	.globl s__BUFFERS
	.globl l__BUFFERS
	.globl s__COMMONMEM
	.globl l__COMMONMEM
	.globl s__COMMONDATA
	.globl l__COMMONDATA
	.globl s__INITIALIZER
	.globl kstack_top

	; exports
	.globl _discard_size

	; startup code
	.area _CODE

;
;	We get booted from the CP/M boot ROM off floppy A:
;	We are loaded at 0x0100 flat with the high things packed
;
	.ascii "NCB"			; The boot ROM expects this magic
start:
	ld	a,#1			; sector
	ld	(0xFC76),a
	ld	hl,#0x0300		; We are in 0100-02FF
	ld	(0xFC82),hl
	ld	a,#0x70			; Sector count (512 bytes per sector)
	ld	(0xFC18),a
next:
	call	0xF003			; Boot helper
	ld	hl,#0xFD0A
	ld	de,(0xFC82)
	ld	bc,#512
	ldir
	ld	(0xFC82),de
	ld	hl,#0xFC76
	inc	(hl)
	ld	a,#10
	cp	(hl)
	jr	nz, same
	ld	(hl),#0
	dec	hl
	inc	(hl)			; Next track (single sided)
same:
	ld	hl,#0xFC18
	dec	(hl)
	jr	nz, next
	; The kernel is now loaded
	ld	sp, #kstack_top
	; move the common memory where it belongs
	ld	hl,#0xE000
	ld	de, #s__COMMONDATA
	ld	bc, #l__COMMONDATA
	ldir

	; then zero the data area
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl), #0
	ldir
	; Zero buffers area
	ld	hl, #s__BUFFERS
	ld	de, #s__BUFFERS + 1
	ld	bc, #l__BUFFERS - 1
	ld	(hl), #0
	ldir
	ld	hl,#s__COMMONMEM
	ld	de,#s__DISCARD
	or	a
	sbc	hl,de
	ld	(_discard_size),hl
	call	init_early
	call	init_hardware
	call	_fuzix_main
	di
stop:	halt
	jr	stop

	.area _DISCARD
_discard_size:
	.dw 0
