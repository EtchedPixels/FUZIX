        .module crt0

	; Loaded at 0x4000 and the lowest available RAM (the display we keep
	; in bank 7 and mapped high).
	.area _COMMONDATA
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
	; These are loaded as low as we can in memory. If we are using an
	; interface 1 cartridge then _CONST to the end of _STUBS can live in
	; ROM. Right now we still need to fiddle with RO as we don't use the
	; IM2 hack and we'll need to modify SDCC and the bank linker not to
	; use RST to avoid this.
        .area _CONST
        .area _COMMONMEM
	.area _STUBS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _INITIALIZER
        .area _GSINIT
        .area _GSFINAL
        .area _CODE
	.area _CODE2
	;
	; Code3 sits above the display area along with the font and video
	; code so that they can access the display easily.
	;
	.area _CODE3
        .area _VIDEO
        .area _FONT
	; Discard is dumped in at 0x8000 and will be blown away later.
        .area _DISCARD

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__INITIALIZER
        .globl l__INITIALIZER
        .globl s__INITIALIZED
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	.include "kernel.def"

        ; startup code
        .area _CODE
init:
        jp 0x003            ; workaround for lowlevel-z80.s check for C3 at 0000
        di

        ; if any button is pressed during reset - boot BASIC48
        in a, (#0xFE)       ; only low 5 bits of 0xFE port contains key info. Bit is 0 when corresponding key of any half row is pressed.
        or #0xE0            ; so setting high 3 bits to 1
        add #1              ; and check if we got 0xFF

        jp z, init_continue

        ; otherwise perform ROM bank switch and goto 0x0000
        ld de, #0x4000
        ld hl, #jump_to_basic_start
        ld bc, #jump_to_basic_start - #jump_to_basic_end
        ldir
        jp 0x4000

jump_to_basic_start:
        ld bc, #0x7FFD
        ld a, #0x10
        out (c), a
        jp 0
jump_to_basic_end:

        ; spacer
        .ds 0x0B

        ; .org 0x0030       ; syscall entry
        jp unix_syscall_entry

        .ds 0x05            ; spacer

        ; .org 0x0038       ; interrupt handler
        jp interrupt_handler
        .ds 0x2B

        ; .org 0x0066       ; nmi handler
        jp nmi_handler

init_continue:
        ; hack for emulator. Read remaining fuzix part to RAM from fuzix.bin
;        ld bc, #0x1ee7
;        in a, (c)
	ld hl, #0x4000
	ld de, #0x4001
	ld (hl), #0
	ld bc, #0x1800
	ldir
	ld (hl), #0x7
	ld bc, #0x02ff
	ldir

;
;	These hooks will be platform/dev specific
;
	.if MICRODRIVE_BOOT
	.globl mdv_boot

boot_stack .equ 0xc000
	ld sp, #boot_stack
	call mdv_boot
	.endif


        ld sp, #kstack_top

        ; Configure memory map
        call init_early

        ; our COMMONMEM is located in main code-data blob, so we
        ; do not need to move it manually

	; initialized data
	ld hl, #s__INITIALIZER
	ld de, #s__INITIALIZED
	ld bc, #l__INITIALIZER
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

	.area _STUBS
stubs:
	.ds 384
