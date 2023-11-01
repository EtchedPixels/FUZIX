;
;	The ROM on this platform is a bit of a pain. It slices the disk
;	into 8MB chunks and boots by loading a 16K image from the start of
;	that slice, but never slice 0. This requires some bizarre boot
;	volume disk partitioning as a result
;
;	We still put the kernel in the low boot area of the device just the
;	bootstrap goes for a bizarre wander.
;

	.abs
;
;	Occupies the entire 'boot track' (16K/32 sectors)
;	Just use the top bit so we can load the image easily
;
	.org 0xC000

start:
	jp high

	.ds 16384 - 512

DATA		.equ 	0x10
ERROR		.equ	0x11
FEATURES	.equ	0x11
COUNT		.equ	0x12
LBA_0		.equ	0x13
LBA_1		.equ	0x14
LBA_2		.equ	0x15
LBA_3		.equ	0x16
STATUS		.equ	0x17
CMD		.equ	0x17

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x20

SIOA_C		.equ	0x02
SIOA_D		.equ	0x00

high:
	ld sp, #0xFE00
	ld hl,#hello
	call serstr

	xor a
	out (0x38),a

	call ide_ready
	ld a,#0xE0
	out (LBA_3),a
	call ide_ready
	xor a
	out (LBA_2),a
	out (LBA_1),a

	ld de,#0x7B02		; sectors 2 to 126
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
	ld a,#'.'
	call serout
	inc e
	dec d
	jr nz, load_loop
	ld hl,#gogogo
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
	and #0x04			; get THRE bit
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
	.byte 13,10,13,10
	.ascii 'SBC2G FUZIX LOADER 0.1'
	.byte 13,10,13,10,0
gogogo:
	.byte 13,10
	.ascii 'Executing FUZIX...'
	.byte 13,10,0

	.org 0xFFFE
	.word start
