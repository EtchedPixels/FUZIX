; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Stuff that ends up in RAM, initialized bits first then data
	; We will need to pull the initialized bits into ROM for crt0.s
	; to ldir down
        .area _COMMONMEM
        .area _CONST
        .area _INITIALIZED
	.area _STUBS
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL
	; Udata ends up just below program code (so we can swap it easily)
	.area _UDATA
	; First ROM is CODE (CODE2 folded in)
        .area _CODE
	; Second ROM is CODE3 VIDEO FONT and initialized data to copy down
	.area _CODE3
	.area _FONT
	.area _VIDEO
        .area _INITIALIZER
	; Discard needs splitting code/data!
	.area _DISCARD
	; and the bitmap display lives at E000-FFFF


        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

        .area _CODE
	; Special headers for the type 'P' ROM

	; Entry 0 is a dummy directory entry holding the volume info
	.db 0xE5		; Empty
	.db 0x50		; Program
	.db 0xA0		; 2 x 32K ROM
	.dw 0xFFFF		; Checksum (but unused) - FIXME set this
	.ascii 'H80'		; Magic string
	.ascii '*FUZIX OS ROM*'
	.db 0x04		; Four directory entries
	.ascii 'V01'		; Version
	.ascii '032615'		; so much for Y2K

	; Entry 1
	.db 0x00		; Directory entry
	.ascii 'FUZIX   COM'	; name of executable
	.dw 0x00		; Extents
	.db 0x00		; Reserved
	.db 0x01		; 1 record
	.db 0x01,0x00,0x00,0x00	; 16 blocks
	.db 0x00,0x00,0x00,0x00
	.db 0x00,0x00,0x00,0x00
	.db 0x00,0x00,0x00,0x00

	; Entry 2
	.db 0xE5
	.ds 31			; 32 bytes per entry
	; Entry 3
	.db 0xE5
	.ds 31

	; Header of first block of "code"
	.db 0xDD		; indicate "run from ROM"
	.db 0xDE
	.db 0xC3, 0x0B, 0x01
	jp coldstart
	jp warmstart
	;
	;	Reference code
	;
	.db 0x0E, 0x1A, 0x11, 0x3A, 0xF9, 0xCD, 0x05, 0x00
	.db 0x21, 0x5C, 0x00, 0x11, 0x5D, 0x00, 0x01, 0x23
	.db 0x00, 0xAF, 0x77, 0xED, 0xB0, 0x21, 0x59, 0x01
	.db 0x11, 0x67, 0x00, 0x0E, 0x0B, 0xED, 0xB8, 0xCD
	.db 0x99, 0xFF, 0x0E, 0x09, 0x11, 0x3E, 0x01, 0xCD
	.db 0x05, 0x00, 0xAF, 0x32, 0x28, 0xEF, 0x0E, 0x00
	.db 0xCD, 0x05, 0x00, 0x52, 0x4F, 0x4D, 0x20, 0x41
	.db 0x43, 0x43, 0x49, 0x44, 0x45, 0x4E, 0x54, 0x20
	.db 0x21, 0x21, 0x07, 0x24
	; Name
	.ascii 'FUZIX   COM'



        ; startup codeinit:
coldstart:
warmstart:			; Until we have suspend/resume done
        ld sp, #kstack_top

        ; Configure memory map
        call init_early

	; zero the data area
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
	.ds 512
