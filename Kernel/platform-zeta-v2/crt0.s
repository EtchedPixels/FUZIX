; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
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
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
        .area _COMMONMEM

        ; exported symbols
        .globl init
        .globl init_from_rom
        .globl _boot_from_rom

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
        .globl kstack_top

	.include "kernel.def"

        ; startup code
        .area _CODE
init:                       ; must be at 0x88 -- warm boot methods enter here
        xor a
        jr init_common
init_from_rom:              ; must be at 0x8B -- bootrom.s enters here
        ld a, #1
        ; fall through
init_common:
        di
        ld (_boot_from_rom), a
        or a
        jr nz, mappedok     ; bootrom.s loads us in the correct pages

        ; move kernel to the correct location in RAM
        ; note that this cannot cope with kernel images larger than 48KB
        ld hl, #0x0000
        ld a, #32               ; first page of RAM is page 32
movenextbank:
        out (MPGSEL_3), a       ; map page at 0xC000 upwards
        ld de, #0xC000
        ld bc, #0x4000          ; copy 16KB
        ldir
        inc a
        cp #35                  ; done three pages?
        jr nz, movenextbank

	; setup the memory paging for kernel
        out (MPGSEL_3), a       ; map page 35 at 0xC000
        ld a, #32
        out (MPGSEL_0), a       ; map page 32 at 0x0000
        inc a
        out (MPGSEL_1), a       ; map page 33 at 0x4000
        inc a
        out (MPGSEL_2), a       ; map page 34 at 0x8000

mappedok:
        ; switch to stack in high memory
        ld sp, #kstack_top

        ; move the common memory where it belongs    
        ld hl, #s__DATA
        ld de, #s__COMMONMEM
        ld bc, #l__COMMONMEM
        ldir
        ; and the discard (which might overlap)
        ld de, #s__DISCARD
        ld bc, #l__DISCARD-1
	add hl,bc
	ex de,hl
	add hl,bc
	ex de,hl
        lddr
	ldd
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
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

_boot_from_rom: .db 0
