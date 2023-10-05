; 2013-12-18 William R Sowerbutts

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
        .area _DISCARD
        .area _COMMONMEM
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
        .globl kstack_top

	.globl _systype
	.globl _syskhz
	.globl _syscpu

	.include "kernel.def"
        .include "../../cpu-z180/z180.def"

	.globl outchar

        ; startup code
	;
	; We are entered at 0x0100 in the first RAM bank with the low 32K
	; sanely mapped but quite possibly not the rest, and with interrupts
	; off.
	;
        .area _CODE
init:
	; Get the ROM info off the stack before we flip banks about
	pop bc			; B = ROMWBW ver C = platform
	pop de			; DE = CPU speed in KHz
	pop hl			; H - Z80 variant L = CPU speed in MHz
        di
	ld a,#0xFE
	out (0x0D),a
        ld sp, #kstack_top

	exx

        ; move the common memory where it belongs    
        ld hl, #s__DATA
        ld de, #s__COMMONMEM
        ld bc, #l__COMMONMEM
        ldir
        ; and the discard
        ld de, #s__DISCARD
        ld bc, #l__DISCARD-1
	add hl,bc
	ex de,hl
	add hl,bc
	ex de,hl
	inc bc
        lddr

        ; then zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

	ld a,#0xFC
	out (0x0D),a
        ; Configure memory map
        call init_early

	ld a,#0xF8
	out (0x0D),a

	; We now have the ROM info to hand and have cleared stuff so it
	; is safe to write these into memory

	exx
	ld (_systype), bc
	ld (_syskhz), de
	ld (_syscpu), hl

        ; Hardware setup
        call init_hardware

	ld a,#0xF0
	out (0x0D),a

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
