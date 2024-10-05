;
;	Bootstrap loader for FUZIX from the ROM
;
;	The ROM thinks it is loading CP/M but is pretty flexible
;

		.module loader

		.area _BOOT(ABS)

		.org 0xFA00
		; Sector 0

DATA		.equ 	0x38
ERROR		.equ	0x39
FEATURES	.equ	0x39
COUNT		.equ	0x3A
LBA_0		.equ	0x3B
LBA_1		.equ	0x3C
LBA_2		.equ	0x3D
LBA_3		.equ	0x3E
STATUS		.equ	0x3F
CMD		.equ	0x3F

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x20

start:
		.word 0x2176		; Magic number
		.word 0xFA00		; Load address
		.byte 0xC9
		.byte 0xC3
		.word 0xFC00		; Run address

		; Sector 1 and 2
		.area _MAIN(ABS)
		.org 0xFC00

run:
		ld sp,#0xFC00
		ld a,#0x80		; ROM off lights off
		out (0xF8),a
		ld hl,#signon
		call print
		
		; Kernel will now get loaded linearly into memory
		; including bankable bank 0

		call ide_ready
		ld a,#0xE0
		out (LBA_3),a
		call ide_ready
		xor a
		out (LBA_2),a
		out (LBA_1),a

		ld de,#0x7C03		; sectors 3-127
		ld hl,#0x0000		; load address
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
		call putchar
		inc e
		dec d
		jr nz, load_loop
		ld hl,#gogogo
		call print
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

print:		ld a,(hl)
		or a
		ret z
		call putchar
		inc hl
		jr print
putchar:	push af
waitch:
		in a,(0x15)
		bit 5,a
		jr z, waitch
		pop af
		out (0x10),a
		ret
signon:
		.asciz '\r\n\r\nSmallZ80 FUZIX LOADER 0.1\r\n'
gogogo:
		.asciz '\r\nExecuting FUZIX...\r\n'
