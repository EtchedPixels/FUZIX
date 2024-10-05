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
	.abs

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
	.ascii 'Fuzix WBW Loader'
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
;	it's has RST 8 interfaces for this rather than just passing
;	them in a register.
;
bootit:
	ld sp, 0xFE00		; SP as high as we can
	ld bc, 0xF8F0		; Get the CPU info into H L and speed into
				; DE
	rst 8

	push hl
	push de

	ld bc, 0xF100		; Get system type into L
	rst 8

	push hl

	ld bc, 0xF8E0		; Get boot sysinfo into DE
	rst 8
	ld b, 0x13		; ROMWBW disk read request
	ld c, d			; Device (same as booted off)
	ld hl, 0x8000		; Loading at 0x8000 for the moment
	ld e, 32		; 32 sectors (16K)
	push bc
	rst 8			; Can error but if so wtf do we do ?
	ld hl, 0x8000
	ld de, 0x0100
	ld bc, 0x4000		; We've loaded 0100-40FF
	ldir
	pop bc
	ld hl, 0x8000
	ld e, 32		; Load the next 16K
	push bc
	rst 8
	ld hl, 0x8000		; Move it into place
	ld de, 0x4100
	ld bc, 0x4000
	ldir
	pop bc
	ld hl, 0x8100		; Now load 8100 up
	ld e, 55		; takes us up to F100. If that's not enough
				; before unpack we have a problem as we
				; will need to move the loader
	rst 8
	jp 0x0100
