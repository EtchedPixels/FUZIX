        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area CODE-CODE2
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
	; Discard is loaded where process memory wil blow it away
        .area _DISCARD
	; The rest grows upwards from C000 starting with the udata so we can
	; swap in one block, ending with the buffers so they can expand up
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
	; These get overwritten and don't matter
        .area _INITIALIZER ; binman copies this to the right place for us
        .area _COMMONMEM

        ; exported symbols
	.globl _suspend

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__COMMONDATA
        .globl l__COMMONDATA
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

	.globl _mach_zrcc

	.include "kernel.def"

	; Starts at 0x1000 at the moment

	; Entered with bank = 1 from the bootstrap logic
	; A holds the machine type: 0 = SBC64/MBC64 1 = ZRCC

resume_sp .equ 0x0080
resume_tag .equ 0x0084

	.area _CODE

	jp start
	jp resume
	.dw 0x10AD

start:
	ld sp, #kstack_top

	; move the common memory where it belongs    
	ld hl, #s__DATA
	ld de, #s__COMMONMEM
	ld bc, #l__COMMONMEM
	ldir
	ld de, #s__COMMONDATA
	ld bc, #l__COMMONDATA
	ldir
	; then the discard
	; Discard can just be linked in but is next to the buffers
	ex de,hl
	ld hl, #s__DISCARD
	ld bc, #l__DISCARD-1
	add hl,bc
	ex de,hl
	add hl,bc
	; May be overlapping its own destination
	lddr
	ldd

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

	ld (_mach_zrcc),a

	call init_hardware
	call _fuzix_main
	; Should never return
	di
stop:	halt
	jr stop
;
;	Helpers for the battery backed memory model
;
resume:
	di
	ld sp, (resume_sp)
	pop iy
	pop ix
	ld hl, #0
	ld (resume_tag), hl
	ret

_suspend:
	push ix
	push iy
	ld (resume_sp), sp
	ld hl,#0xC0DE
	ld (resume_tag), hl
	ld hl, #suspended
	call outstring
	di
	halt
suspended:
	.ascii 'Suspended, you may now power off'
	.db 13,10,0


	