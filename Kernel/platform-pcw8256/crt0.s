	; Ordering of segments for the linker.
	; WRS: Note we list all our segments here, even though
	; we don't use them all, because their ordering is set
	; when they are first seen.	
	.area _CODE
	.area _CODE2
	.area _VIDEO
	.area _CONST
	.area _INITIALIZED
	.area _DATA
	.area _BSEG
	.area _BSS
	.area _HEAP
	; note that areas below here may be overwritten by the heap at runtime, so
	; put initialisation stuff in here
	.area _GSINIT
	.area _GSFINAL
	.area _DISCARD
	.area _INITIALIZER
	.area _FONT
	.area _COMMONMEM

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
	.globl kstack_top
	.globl start

	.globl _int_disabled

	; startup code
	.area _CODE
;
;	We have 0x88 bytes before the standard FUZIX start point, everyone
;	breathe in
;
;	Once the first loader block is called we are called via RST 0 each
;	block end and we do the gui progress bar bits that wouldn't fit in
;	the boot block
;
;	On entry h is set to 0 and the top of screen is banked at 0x8000
;
track	.equ 0xF1F4

progress:
	; top bar
	; bank then down 14 character lines
	; and 
	ld	de, #(0x8000+0x2760+312)
	ld	a, #font0
	rst	0x18	; choline
	; progress bar
	ld	de, #(0x8000+0x2760+312+720)
	rst	0x18	; choline
	ld	de, #(0x8000+0x2760+312+720+8)
	ld	a, (track)
	inc	a
	ld	b, a
	ld	a, #fontX
	rst	0x28	; nchout
	; bottom bar
	ld	de, #(0x8000+0x2760+312+2*720)
choline:rst	0x20	; chout	at 0x18
	add	a,#8
	ld	b, #10
	rst	0x28	; nchout
	jr	tail
chout:	ld	l, a		; chout at 0x20
	ld	bc, #8
	ldir
	ret
	nop		; and nchout at 0x28
nchout:	push	bc
	rst	0x20
	pop	bc
	djnz	nchout
tail2:	add	a, #8
	ret
tail:	rst	0x20
	jr	tail2

font0:	.db	0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
font1:	.db	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
font2:	.db 	0x00, 0x00, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8

font3:	.db	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
font4:	.db	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00
font5:	.db	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8

fontX:	.db	0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00

font6:	.db	0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00
font7:	.db	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00
font8:	.db 	0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x00

	.dw	0, 0
		; Pad to 0x88 bytes
	.db	0
;
;	Once the loader completes it jumps here
;
;	On entry - stack is somewhere high, video is in bank 4 to middle of
; bank 5 and the OS is loaded into banks 0-2
;
start:
	; Joyce tracing on
;	ld	a,#0x80
;	.dw	0xfeed
	ld	sp, #kstack_top
	;
	;	Move the common into place (our build tool
	;	moved INITIALIZED ready and then packed common after
	; 	it)
	;
	ld	hl, #s__DATA
	ld	de, #s__COMMONMEM
	ld	bc, #l__COMMONMEM
	ldir
	;
	;	And the data
	;
	ld	de, #s__COMMONDATA
	ld	bc, #l__COMMONDATA
	ldir
	;
	;	Straight after that is the font. We need the font
	;	somewhere accessible when the display is mapped sp
	;	put it after the display. hl points at the font data
	;	need to copy it from common
	;
	call	fontcopy
	;
	;	This is followed by the DISCARD area. In theory
	;	this could self overlap if it grows enough
	;
	ld	de, #s__DISCARD
	ld	bc, #l__DISCARD - 1
	add	hl,bc
	ex	de,hl
	add	hl,bc
	ex	de,hl
	lddr
	ldd

	;
	;	Zero the data area
	;
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl), #0
	ldir

	ld	a,#1
	ld	(_int_disabled),a
	call	init_early
	call	init_hardware
	call	_fuzix_main
	di
stop:	halt
	jr	stop

;
;	Keep this linked low as we will unmap a chunk of kernel to font copy
;
fontcopy:
	ld	 a, #0x85
	out	(0xf1), a	; font is linked high so clear of 16-32K
	ld	de, #23552	; after roller ram
	ld	bc, #2048	; 256x8 font
	ldir
	ld	a, #0x81	; put the kernel back
	out	(0xf1), a
	ret			; and return into it
