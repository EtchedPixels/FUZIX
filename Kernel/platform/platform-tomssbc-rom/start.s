		.area _BOOT(ABS)

		.module start

		.globl unix_syscall_entry
		.globl interrupt_handler
		.globl _go

		.globl s__CONST
		.globl l__CONST
		.globl l__STUBS
		.globl s__INITIALIZER
		.globl l__INITIALIZER
		.globl s__COMMONMEM
		.globl l__COMMONMEM
		.globl l__COMMONDATA

		.globl rst30
;
;	This chunk is placed into 0x0000-0x007F of all banks
;

		.org 0

rst0:		jp next	;	Has to be JP to avoid triggering debug checks
next:		di
		; ROM is on and we are in bank 0
		ld a,#0x01
		out (0x3E),a
		xor a
		out (0x3F),a
		; now continuing in the copy in bank 2 (CODE3)
		; where the top half is packed data
		; for runtime
		ld hl,#0x2000
		ld de,#0x4000
unpack:
		ld a,(hl)
		or a
		jr z,block
		cp #255
		jr z,block
		ldi
		jr unpack
block:		ld c,a
		inc hl
		ld a,(hl)
		cp #254
		jr nc,big
		ld b,a
		ld a,c
		inc b
doblock:
		ld (de),a
		inc de
		djnz doblock
		inc hl
		jr unpack

		nop
		nop
		
rst30:		jp unix_syscall_entry
		nop
		nop
		nop
		nop
		nop
rst38:		jp interrupt_handler

big:		; Big block
		; A is 254 or 255, C is the char, 254 = big block, 255 =
		; done
		inc a
		jr z, runit
		inc hl
		ld a,c
		ld c,(hl)
		inc hl
		ld b,(hl)
		inc hl
		inc bc
		ex de,hl
		ex af,af'
bigloop:
		ex af,af'
		ld (hl),a
		inc hl
		dec bc
		ex af,af'
		ld a,b
		or c
		jr nz,bigloop
		ex de,hl
		jr unpack
runit:
		; On real Z80 silicon SP always comes up as FFFF but it's
		; not officially written down that this is so
		ld sp,#0xffff
		push af
		call _go
		pop af
		di
		halt
