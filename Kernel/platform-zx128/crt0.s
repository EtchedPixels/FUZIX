; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _DATA
        .area _COMMONMEM
        .area _CODE2
        .area _VIDEO
        .area _DISCARD      ; not discarded yet
        .area _CONST
        .area _FONT
        .area _INITIALIZED
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _INITIALIZER
        .area _GSINIT
        .area _GSFINAL

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top


        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

        ; startup code
        .area _CODE
init:
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
        .ds 0x0E

        ; .org 0x0030       ; syscall entry
        jp unix_syscall_entry

        .ds 0x05            ; spacer

        ; .org 0x0038       ; interrupt handler
        jp interrupt_handler
        .ds 0x2B

        ; .org 0x0066       ; nmi handler
        jp nmi_handler

init_continue:
        ld sp, #kstack_top

        ; hack for emulator. Read remaining fuzix part to RAM from fuzix.bin
        ld bc, #0x1ee7
        in a, (c)

        ; Configure memory map
        call init_early

        ; move the common memory where it belongs    
        ld hl, #s__INITIALIZER
        ld de, #s__COMMONMEM
        ld bc, #l__COMMONMEM
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

