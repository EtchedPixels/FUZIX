;
;	The CP/M command blindly loads the first 24 blocks moves them to the
;	top 12K and then runs whatever is indicated at 0xFFFE/FFFF
;

	.z180

	.include "kernel.def"
        .include "../../cpu-z180/z180.def"

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

	.ds 10240	; boot sector and more for loader space

start:
	; A modern partition setup has lots of room for boot space, so we
	; just load the blocks after the loader (24-147). We can't overlap
	; because of the FFFE/FFFF thing.

	ld sp, #0xFE00
	ld hl,#hello
	call serstr

	in0 a,(MMU_BBR)
	call outhex
	in0 a,(MMU_CBR)
	call outhex
	in0 a,(MMU_CBAR)
	call outhex

	; We are above 0x8000 (CBAR) so make BBR = CBR instead of 
	; having to self relocate

	in0 a,(MMU_CBR)
	out0 (MMU_BBR),a

	; Kernel will now get loaded linearly in the top 64K
	; for the current ROM anyway

	call ide_ready
	ld a,#0xE0
	out (LBA_3),a
	call ide_ready
	xor a
	out (LBA_2),a
	out (LBA_1),a

	ld de,#0x7C18		; sectors 24-119
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


serstr:
	ld a,(hl)
	or a
	ret z
	inc hl
	call serout
	jr serstr

; print the byte in A as a two-character hex value
outhex:
        push bc
	push af
        ld c, a  ; copy value
        ; print the top nibble
        rra
        rra
        rra
        rra
        call outnibble
        ; print the bottom nibble
        ld a, c
        call outnibble
	pop af
        pop bc
        ret

; print the nibble in the low four bits of A
outnibble:
        and #0x0f ; mask off low four bits
        cp #10
        jr c, numeral ; less than 10?
        add a, #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:add a, #0x30 ; start at '0' (0x30='0')

serout:
	push af
	; wait for transmitter to be idle
ocloop_sio:
	in0 a,(ASCI_STAT0)		; read Line Status Register
	and #0x02			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out0 (ASCI_TDR0),a
	ret


hello:
	.asciz 'SC111 FUZIX LOADER 0.1\r\n\r\n'
gogogo:
	.asciz '\r\nExecuting FUZIX...\r\n'
