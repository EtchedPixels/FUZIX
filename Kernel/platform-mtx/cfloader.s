DATA		.equ 	0xB0
ERROR		.equ	0xB1
FEATURES	.equ	0xB1
COUNT		.equ	0xB2
LBA_0		.equ	0xB3
LBA_1		.equ	0xB4
LBA_2		.equ	0xB5
LBA_3		.equ	0xB6
STATUS		.equ	0xB7
CMD		.equ	0xB7

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x20

	.area LOADER(ABS)

	.org 0xFE00

start:
	di
	ld a,#'B'
	out (0x60),a
	ld hl, #0x0100
	ld de, #0xFE00
	ld bc, #0x0200
	ldir
	jp go			; Into the high space
go:
	ld a,#'O'
	out (0x60),a

	ld a,#0x80
	out (0),a

	ld sp, #0xFE00
	ld a,#'O'
	out (0x60),a
	; Kernel will now get loaded linearly in the top 64K

	call ide_ready
	ld a,#0xE0
	out (LBA_3),a
	call ide_ready
	xor a
	out (LBA_2),a
	out (LBA_1),a

	ld de,#0x7C01		; sectors 1+
	ld hl,#0x0100		; load address
load_loop:
	call ide_ready
	ld a,e
	out (LBA_0),a
	ld a,#1
	out (COUNT),a
	ld a,#ATA_READ
	out (CMD),a
	nop
	call ide_wait_drq
	ld bc,#DATA
	inir
	inir
	inc e
	dec d
	jr nz, load_loop
	ld a,#'T'
	out (0x60),a
	jp 0x0100

ide_ready:
	in a,(STATUS)
	bit BUSY,a
	jr nz, ide_ready
	bit READY,a
	jr z, ide_ready
	ret
ide_wait_drq:
	in a,(STATUS)
	bit DRQ,a
	jr z,ide_wait_drq
	ret
