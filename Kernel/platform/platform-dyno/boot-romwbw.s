;
;	ROMWBW boot block
;
;	The provided bootloader is a bit limited. It can only really load
;	stuff into upper memory and assumes CP/M is being loaded
;
;	Unfortunately this means we have to chain our own loader.
;	Fortunately ROMWBW is really quite nice so it's easy to use to do
;	the load. What it can't do however is do I/O into the low 32K
;	directly - it has no separate I/O target page feature it seems.
;
;	We build the lot as if it's a binary at 0xF000 as that's easier than
;	fighting the linker and the like
;
;	The end must actually be the byte after we need. ROMWBW explodes
;	if given F200 F3FF !
;
;	ROMWBW 2.9.1 has an undocumented antisocial habit - it enables
;	interrupts even if called with them disabled. Work around this
;	by turning them back off when we need too
;
		.module boot

.area	    BOOT	(ABS)
	.org	    0xF000

	.ds   384
	.byte 0x5A
	.byte 0xA5
	.byte 0x00	; formatting platform
	.byte 0x00	; device
	.byte 0x00	; formatter
	.byte 0x00	; physical drive
	.byte 0x00	; logical unit
	.byte 0x00	; unused
	.ds 88		; move to byte 96
	.byte 0x00	; write protect
	.word 0x00	; update count
	.byte 0		; major
	.byte 0		; minor
	.byte 0		; update
	.byte 0		; patch
	.ascii 'Fuzix DynoLoader'
	.byte '$'	; indicate valid label
	.word 0		; no patch
	.word 0xF200	; load target
	.word 0xF400	; end - way more than we need but that's fine
	.word 0xF200	; run from here

;
;	This should be 0xF200. We are entered with the stack somewhere
;	in the middle of RAM, HBIOS stubs in the top 2 pages and RAM
;	low including our expected RST hooks for HBIOS
;
;	We need to go get our device/subunit back from the BIOS because
;	it has RST 8 interfaces for this rather than just passing
;	them in a register.
;
;	For now we can do a simple load as we fit in 64K easily. If we
;	add networking we might have to get clever.
;
bootit:
	ld a,#0x01
	out (0x0D),a

	ld bc, #0x0100
	ld e, #13
	rst 8
	ld bc, #0x0100
	ld e, #10
	rst 8

	ld sp, #0xFE00		; SP as high as we can
	ld bc, #0xF8F0		; Get the CPU info into H L and speed into
				; DE
	rst 8

	push hl
	push de

	ld bc, #0xF100		; Get system type into L
	rst 8

	push hl

	ld bc, #0xF8E0		; Get boot sysinfo into DE
	rst 8

	ld a,#0x03
	out (0x0D),a

	ld b, #0x13		; ROMWBW disk read request
	ld c, d			; Device (same as booted off)
	ld hl, #0x8000		; Loading at 0x8000 for the moment
	ld e, #32		; 32 sectors (16K)
	push bc
	rst 8			; Can error but if so wtf do we do ?
	or a
	jr nz,failed

	ld a,#0x07
	out (0x0D),a

	ld hl,#0x8100		; Don't copy first 256 bytes over
	ld de,#0x0100
	ld bc,#0x3F00		; We've loaded 0100-4000
	ldir

	ld a,#0x0F
	out (0x0D),a

	;
	;	Same process again for second 16K of kernel
	;
	pop bc
	ld hl,#0x8000
	ld e,#32		; Load the next 16K
	push bc
	rst 8
	or a
	jr nz,failed

	ld a,#0x1F
	out (0x0D),a

	ld hl,#0x8000		; Move it into place
	ld de,#0x4000
	ld bc,#0x4000
	ldir

	ld a,#0x3F
	out (0x0D),a
	;
	;	And the third 16K
	;
	pop bc
	ld hl,#0x8000		; Low the next 16K
	ld e, #32
	push bc
	rst 8
	or a
	jr nz,failed

	; No need to put it in place


	ld a,#0x7F
	out (0x0D),a
	;
	;	Final 16K
	;
	pop bc
	ld hl,#0xC000		; Now load 0xC000
	ld e, #24		; takes us up to EFFF
	rst 8
	or a
	jr nz,failed

	; Into the kernel

	ld a,#0xFF
	out (0x0D),a
	di
	jp 0x0100

failed:
	ld bc,#0x0100
	ld e,#'E'
	rst 8
	di
	ld a,#0xF0
flasher:
	out (0x0D),a
	ld bc,#0x0020
wait:
	djnz wait
	dec c
	jr nz, wait
	ld c,#0x20
	cpl
	jr wait
