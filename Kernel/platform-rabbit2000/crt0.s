        .module crt0

	; Segment order
	; Note: the code segments may be in flash
        .area _CODE
        .area _CODE2
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _INITIALIZER
        .area _GSINIT
        .area _GSFINAL
	.area _DISCARD
        .area _COMMONMEM

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
	.globl _kdataseg

        ; startup code
        .area _CODE

	.include "kernel.def"
	.include "../kernel-rabbit.def"
;
;	For a RAM startup our configuration is
;
;	Interrupts off
;	Stack undefined
;
;	DATASEG set from 0-DFFF
;	XPC segment undefined but will be somewhere in the bootstrap
;	as it will jump to 0
;
;	Physically mapped so that the kernel starts at ram address 0x0000
;
;	Serial console is configured, wait states and cs are set up validly
;
init:
        ld sp, #kstack_top

	; Learn our banks

	; Will need extending when we start to run from flash
	ioi
	ld a,(DATASEG)
	ld (_kdataseg),a
	add a,#14		; 56K on (our E000)
	ld xpc,a		; We now have 64K linear physical
				; space		
	; For now we don't move XPC, that will change eventually

	;
	; our stack pointer is now valid (we can't do the above in
	; init_early as we have no sane stack)
	;

        ; Configure memory map
        call init_early

	; move the common memory where it belongs (our XPC is now valid) 
	ld hl, #s__DATA
	ld de, #s__COMMONMEM
	ld bc, #l__COMMONMEM
	call doldir
	; and the discard
	ld de, #s__DISCARD
	ld bc, #l__DISCARD
	call doldir
	; then zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	call doldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
stop:   jr stop

doldir:
	; the ldir instruction is broken in Rabbit 2000 series CPU
	; in differing ways depending upon the chip version. So just
	; skip using it.
	ldi
	jp lo,doldir
	ret
