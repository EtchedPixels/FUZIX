;
;	This assumes you are using the ROM CP/M build with banked memory
;	helpers in high RAM.
;

	.abs

	.org 0xFE00

DATA		.equ 	0x90
ERROR		.equ	0x91
FEATURES	.equ	0x91
COUNT		.equ	0x92
LBA_0		.equ	0x93
LBA_1		.equ	0x94
LBA_2		.equ	0x95
LBA_3		.equ	0x96
STATUS		.equ	0x97
CMD		.equ	0x97

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x20

SIOA_C		.equ	0x01
SIOA_D		.equ	0x00


	.byte 'B'
	.byte 'O'
	.byte 'O'
	.byte 'T'
	.byte 'Z'

start:
	; A modern partition setup has lots of room for boot space, so we
	; just load the blocks after the loader (2-126).

	; FFxx we must not touch initially as it holds the banked memory
	; helpers placed by the firmware.

	ld hl, 0x0100
	ld de, 0xFE00
	ld bc, 0x0100
	ldir
	jp go
go:
	ld sp, #0xFE00
	ld hl, hello
	call serstr

	call ide_ready
	ld a, 0xE0
	out (LBA_3),a
	call ide_ready
	xor a
	out (LBA_2),a
	out (LBA_1),a

	ld de, 0x7C02		; sectors 2-126
	ld hl, 0x0100		; load address
load_loop:
	call ide_ready
	ld a,e
	out (LBA_0),a
	ld a, 1
	out (COUNT),a
	ld a, ATA_READ
	out (CMD),a
	nop
	call ide_wait_drq
	ld bc, DATA
	inir
	inir
	ld a, '.'
	call serout
	inc e
	dec d
	jr nz, load_loop
	ld hl, gogogo
	call serstr
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


serout:
	push af
	; wait for transmitter to be idle
ocloop_sio:
        xor a                   ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)		; read Line Status Register
	and 0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (SIOA_D),a
	ret

serstr:
	ld a,(hl)
	or a
	ret z
	inc hl
	call serout
	jr serstr

hello:
	.ascii 'SIMPLE 80 FUZIX LOADER 0.1'
	.byte 13,10,13,10,0
gogogo:
	.byte 13,10
	.ascii 'Executing FUZIX...'
	.byte 13,10,0
