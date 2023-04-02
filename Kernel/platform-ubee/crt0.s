	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE2
		.area _HOME
		; Load video later on so it ends up above 0x8800
		.area _VIDEO
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _GSINIT
	        .area _GSFINAL
		.area _BUFFERS
		.area _DISCARD
	        .area _COMMONMEM
	        .area _COMMONDATA
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _INITIALIZER

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
	        .globl s__COMMONDATA
	        .globl l__COMMONDATA
		.globl s__INITIALIZER
	        .globl kstack_top
		.globl map_kernel

		.globl _ubee_model

	        ; startup code
	        .area _CODE

;
;	Once the loader completes it jumps here. We are loaded between
;	low memory and DFFF. Above us at this moment is ROM and VRAM is
;	at F800 overlapping the top of RAM
;
;	There are lots of uBee's we might have been loaded on but we
;	only have to care about those that have enough memory (128K+_
;
		.word 0xC0DE
start:
		di
		; We can't use the kstack yet - we've still got video mapped
		; all over it
		ld sp, #0x0200
		;
		; Figure out our hardware type. We need to work this out
		; before we can shuffle the memory map and set up video
		;
		; Start with the easy case (we expect our loader to preserve
		; these - we might need to move the model detection and
		; support check into the loader
	        ld a, (0xEFFD)
		cp #'2'
	        jr nz, not256tc
		ld a, (0xEFFE)
		cp #'5'
	        jr nz, not256tc
		ld a, (0xEFFF)
		cp #'6'
	        jr nz, not256tc
		ld a,#2
		ld (_ubee_model),a
		; Do we need to touch ROM etc - not clear we need do
		; anything as we are already in RAM mode. Turn off video
		; mapping
		ld a,#0x0C
		out (0x50),a
		jr relocate

not256tc:	; Are we a premium model
		ld a,#0x10		; Attribute latch
		out (0x1c),a		; Set
	        in a,(0x1c)		; Read
	        cp #0x10		; If reads back we are premium
		ld a,#1
		jr z, set_model
		dec a
		; Colour ?
		ld hl,#0xFFFF		; Out of sight end of video
		out (0x08),a		; Select text
		ld (hl),a		; Clear it
		ld a,#0x40		; Select colour (no-op if absent)
		out (0x08),a
		ld (hl),a		; Set it
		xor a
		out (0x08),a		; Back to text
		cp (hl)			; Was it affected ?
		jr nz, unsupported_model ; Yes -> no colour support
set_model:
		ld (_ubee_model),a	; model - 1 premium 0 colour standard
		ld a,#0x0C		; ROMs off video off
		out (0x50),a

		; FIXME: support SBC (128K non premium 5.25 or 3.5
		; drives, without colour flicker on video writes)
		; may also have a 1793 not 2793 fdc

relocate:
		ld sp,#kstack_top
		;
		; move the common memory where it belongs    
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		ld de, #s__COMMONDATA
		ld bc, #l__COMMONDATA
		ldir
		; then the discard
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
		ldir
		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir

		call map_kernel

		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

;
;	We still have ROM at this point
;
unsupported_model:
	        ld hl,#unsupported_txt
	        call 0xE033
	        jp 0xE000
unsupported_txt:
		.ascii "Unsupported platform"
		.byte 0x80

_ubee_model:	.byte 0
