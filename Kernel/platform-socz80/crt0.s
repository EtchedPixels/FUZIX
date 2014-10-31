; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _CODE2
	.area _DISCARD
        .area _CONST
        .area _DATA
        .area _INITIALIZED
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _INITIALIZER
        .area _GSINIT
        .area _GSFINAL
        .area _COMMONMEM

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl l__INITIALIZER
        .globl s__INITIALIZED
        .globl s__INITIALIZER
        .globl s__BSS
        .globl l__BSS
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top
        .globl _trap_monitor

        ; startup code
        .area _CODE
init:
        di
        ld sp, #kstack_top

        ; Configure memory map
        call init_early
    
        ; Initialise global variables, heap, etc
        call gsinit

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

        ; define function to copy _INITIALIZER to _INITIALIZED
        .area _GSINIT
gsinit::
        ld bc, #l__INITIALIZER
        ld a, b
        or a, c
        jr Z, gsinit_next
        ld de, #s__INITIALIZED
        ld hl, #s__INITIALIZER
        ldir
gsinit_next:
        ; other module's code is appended here

        .area _GSFINAL
        ; we clear _DATA and _BSS
        ld bc, #l__BSS
        ld de, #s__BSS
        call zeroarea
        ld bc, #l__DATA
        ld de, #s__DATA
        call zeroarea
        ret

zeroarea:
        ; zero the memory at (DE) for BC bytes
        ; check BC!=0
        ld a, b
        or c
        ret z
        push de
        pop hl
        ld (hl), #0 ; place initial 0
        inc de
        dec bc
        ; check BC!=0
        ld a, b
        or c
        ret z
        ldir
        ret
