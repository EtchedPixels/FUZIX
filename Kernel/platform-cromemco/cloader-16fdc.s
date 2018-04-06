		.area ASEG(ABS)
		.org  0x80
;
;	Loader for Fuzix on a Cromenco 16FDC
;
;	The kernel is written in blocks 1+ SS/SD with no magic
;	involved. We have 128 bytes, but fortunately a ROM helper
;
;
;	Memory map
;	0x0000-0x002E	Stack
;	0x0060-0x007F	Used by the boot ROM
;	0x0080-0x00FF	This boot block
;	0x0100-0xBFFF	Free RAM
;	0xC000-0xCFFF	Boot ROM (until we out to 0x40)
;	0xD000-0xFFFF	May be RAM but may also be ROM shadows
;

set_side	.equ	0xC003
set_track	.equ	0xC006
set_sector	.equ	0xC009
set_buffer	.equ	0xC00C
disk_restore	.equ	0xC00F
disk_seek	.equ	0xC012
disk_read	.equ	0xC015
disk_write	.equ	0xC018
setup_uart	.equ	0xC01B
char_input	.equ	0xC01E
char_ready	.equ	0xC021
line_input	.equ	0xC024
char_output	.equ	0xC027
newline_output	.equ	0xC02A

	.globl start
	.globl endc
	.globl end
	.globl msg
;	Carry set, drive in A
start:
	push af
	ld hl,#boot
	call tout
	ld hl,#0xFC
	pop af
	ld (hl),a	; boot drive
	inc hl
	ld a,#16
	ld (hl),a	; controller type (16FDC)
	inc hl
	ld bc,(0x7E)	; features and feature save for boot disk
	ld (hl),c
	inc hl
	ld (hl),b
	inc hl		; now 0x100
	xor a
	ld b,a
	call set_side
	ld a,(0x6E)	; sectors per track
	inc a
	ld c,a
	ld a,#1		; sector 2
	ld de,#128
trackloop:
	push af
	ld a,b
	call set_track
	pop af
secloop:
	inc a
	call set_sector
	call set_buffer
	ex af,af'	; shorter than pushing them
	exx
	call disk_seek
	call disk_read
	jr c, crap
	exx
	add hl,de
	ld a,h
	cp #0xC0
	jr z,go
	ex af,af'
	cp c
	jr nz, secloop
	inc b
	xor a
	jr trackloop
go:
	ld a,(0x100)
	cp #0xC3
	jr z, 0x100
crap:
	ld hl, #fail
	call tout
	jp 0xC000

tout:
	ld a,(hl)
	inc hl
	or a
	ret z
	push hl
	call char_output
	pop hl
	jr tout

msg:
boot:	.asciz 'Boot'
fail:	.ascii ' failed'
	.byte 13,10,0

endc:
	.org 0xF8

	.ascii 'FZSSSD'
end:
