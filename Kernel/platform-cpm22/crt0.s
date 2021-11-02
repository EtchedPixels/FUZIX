;
;	We are loaded by whatever loader was written for the platform. We
;	are started in bank 0. We might have been loaded by a loader or by
;	CP/M 2.x
;
        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
	.area _START
        .area _CODE
        .area _CODE2
	.area _HOME
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
	.area _BUFFERS
	.area _DISCARD
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _GSINIT
        .area _GSFINAL
        .area _COMMONMEM
	.area _COMMONDATA
        .area _INITIALIZER

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
        .globl s__SYSMOD
        .globl l__SYSMOD
	.globl s__GSFINAL
        .globl kstack_top

	; system configuration

        .area _CODE

start:
	jr	go
	.word	0x1DEA		; magic for config check
	.word	s__DATA-0x100	; pointer to where to stuff sysmod
go:
        di
        ld	sp, #kstack_top


        ; Configure memory map
        call	init_early

	; move the common memory where it belongs
	; as we need this to boot further
	; this wipes the stack we are using but that is ok

	ld	hl, #s__DATA
	ld	de, #s__SYSMOD
	ld	bc, #l__SYSMOD
	ldir
	ld	de, #s__COMMONMEM
	ld	bc, #l__COMMONMEM
	ldir
	; and the discard
	ld	de, #s__DISCARD
	ld	bc, #l__DISCARD
	ldir

	; then zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	ldir

        ; Hardware setup
        call init_hardware
        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop
