;
;	The CP/M command blindly loads the first 24 blocks moves them to the
;	top 12K and then runs whatever is indicated at 0xFFFE/FFFF
;

	.area VECTOR(ABS)

	.org 0xFFFE
	.word start

	.area LOADER(ABS)

	.org 0xD000

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

SIOA_C		.equ	0x80
SIOA_D		.equ	0x81

	.ds 10240	; boot sector and more for loader space

start:
	; A modern partition setup has lots of room for boot space, so we
	; just load the blocks after the loader (24-147). We can't overlap
	; because of the FFFE/FFFF thing.

	ld sp, #0xFE00
	ld hl,#hello
	call serstr

	call ide_ready
	ld a,#0xE0
	out (LBA_3),a
	call ide_ready
	xor a
	out (LBA_2),a
	out (LBA_1),a

	ld de,#0x6418		; sectors 24-119
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
	.asciz 'SC108 FUZIX LOADER 0.2(SIO)\r\n\r\n'
gogogo:
	.asciz '\r\nExecuting FUZIX...\r\n'
