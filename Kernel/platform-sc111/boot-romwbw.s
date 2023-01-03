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
	.z180
	.include "kernel.def"

	.module	    boot

	.area	    BOOT(ABS)
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
	.ascii 'Fuzix 111 Loader'
	.byte '$'	; indicate valid label
	.word 0		; no patch
	.word 0xF200	; load target
	.word 0xF400	; end - way more than we need but that's fine
	.word 0xF200	; run from here

;
; In order to handle FUZIX IO base correctly, we need to know the
; ROMWBW IO base, which seems to always have been 0xC0 for ROMs
; shipped with SC111.
;
ROMWBW_IO_BASE	.equ 0xC0

;
; SC111 does not have LED status output on a fixed port. So make debug
; output configurable.
;
DEBUGENABLE	.equ 0
DEBUGPORT	.equ 0x00

.if DEBUGENABLE
.macro statusled VALUE
	ld a,VALUE
	out (DEBUGPORT),a
.endm
.else
.macro statusled VALUE
	;; do nothing
.endm
.endif

.macro bootinfo			; Call ROMWBW/HBIOS to get boot device id
	ld bc, #0xF8E0		; SYSGET functions, BOOTINFO subfunction
	rst 8			; Call HBIOS
	ld b, #0x13		; Prepare B for diskload macro, function 0x13 - Disk Read
	ld c, d			; Prepare C for diskload macro, device unit
	push bc			; push BC to stack to be used by diskload macro
.endm

.macro diskload DEST,BLOCKS,FAIL; Call ROMWBW/HBIOS to load next #BLOCKS of 512 bytes
	ld hl, DEST		; HL contains address of destination buffer
	ld e, BLOCKS		; E contains block count
	pop bc			; Get function + device unit from stack
	push bc			; Push function/device to stack for next invocation
	rst 8			; Call HBIOS
	or a			; Check success status
	jp nz,FAIL		; Jump to FAIL in case of error
.endm

.macro memcopy DEST,SOURCE,SIZE ; Copy SIZE bytes from SOURCE to DEST
	ld hl,SOURCE
	ld de,DEST
	ld bc,SIZE
	ldir
.endm

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
	statusled #0x01

	ld sp, #0xFE00		; SP as high as we can

	ld hl, #hello
	call outstr

	bootinfo

	;; First 16K block loads from disk offset 0x0800
	;; with kernel starting at disk offset 0x3000
	diskload #0x8000,#32,failed
	statusled #0x03
	memcopy #0x0100,#0xA800,#0x1800
	statusled #0x07

	;; Second 16K block to 0x1900
	diskload #0x8000,#32,failed
	statusled #0x0F
	memcopy #0x1900,#0x8000,#0x4000
	statusled #0x1F

	;; Third 16K block to 0x5900
	diskload #0x8000,#32,failed
	statusled #0x3F
	memcopy #0x5900,#0x8000,#0x4000
	statusled #0x7F

	;; Last 16K block, directly loaded to 0x9900
	diskload #0x9900,#32,failed
	statusled #0xFF

	call printiobase
	di

.if ROMWBW_IO_BASE-Z180_IO_BASE
	;; when ROMWBW_IO_BASE and Z180_IO_BASE are different, set ICR
	;; to Z180_IO_BASE. Do this as late as possible as HBIOS calls
	;; do not to work anymore after IO base switch.
	ld a, #Z180_IO_BASE
	out0 (ROMWBW_IO_BASE+0x3F), a
.endif

	jp 0x0100

failed:
	ld hl, #error
	call outstr
	di

.if DEBUGENABLE
	ld a,#0xF0
flasher:
	out (DEBUGPORT),a
	ld d, #0x08
	ld c, #0x00
	ld b, #0x00
wait:
	djnz wait
	dec c
	jr nz, wait
	dec d
	jr nz, wait
	cpl
	jr flasher
.else
	halt
.endif

; Print I/O base status line like this:
; ROMWBW IO=0xC0 FUZIX IO=0xC0
printiobase:
	ld hl, #romlabel
	call outstr
	ld a, #ROMWBW_IO_BASE
	call outhex
	ld hl, #fuzixlabel
	call outstr
	ld a, #Z180_IO_BASE
	call outhex
	ld hl, #newline
	call outstr
	ret

; print zero-terminated string pointed to by HL
outstr:
	ld a,(hl)
	or a
	ret z
	inc hl
	ld bc, #0x0100
	ld e, a
	rst 8
	jr outstr

; print the byte in A as a two-character hex value
outhex:
	push af
	; print the top nibble
	rra
	rra
	rra
	rra
	call outnibble
	; print the bottom nibble
	pop af
	call outnibble
	ret

; print the nibble in the low four bits of A
outnibble:
	and #0x0f ; mask off low four bits
	cp #10
	jr c, numeral ; less than 10?
	add a, #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:add a, #0x30 ; start at '0' (0x30='0')
	ld e, a
	ld bc, #0x0100
	rst 8
	ret

hello:
	.asciz '\r\nSC111 ROMWBW FUZIX LOADER 0.1\r\n'
romlabel:
	.asciz 'ROMWBW IO=0x'
fuzixlabel:
	.asciz ' FUZIX IO=0x'
newline:
	.asciz '\r\n\r\n'
error:
	.asciz 'ERROR\r\n'
