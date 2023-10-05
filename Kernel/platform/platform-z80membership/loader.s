;
;	Bootstrap loader for FUZIX from the ROM
;
;	The ROM thinks it is loading CP/M but is pretty flexible
;

		.module loader

		.area _BOOT(ABS)

		.org 0xF800

start:
		di
		ld sp,#0xF800
		ld hl,#signon
		call print

		xor a
		out (0xC1),a		; ROM back in
		
		; Kernel will now get loaded linearly into memory
		; including bankable bank 0

		ld de,#0x7B03		; sectors 3-126
		ld hl,#0x0100		; load address
load_loop:
		call sd_load_1
		ld a,#'.'
		call putchar
		inc e
		dec d
		jr nz, load_loop
		ld hl,#gogogo
		call print
		ld a,#1
		out (0XC1),a		; ROM back out
		jp 0x0100

print:		ld a,(hl)
		or a
		ret z
		call putchar
		inc hl
		jr print
putchar:	push af
waitch:
		in a,(0xCD)
		bit 5,a
		jr z, waitch
		pop af
		out (0xC8),a
		ret

sd_load_1:
		push bc
		push de
		push hl
		ld bc,#0
		ld d,c
		call 0x24AF	; load sector BCDE
		pop hl
		ex de,hl	; DE is now target
		ld hl,#0xFC00	; SD RAM buffer
		ld bc,#0x0200
		ldir		; to the right place
		ex de,hl	; HL is now next target
		pop de
		pop bc
		ret
signon:
		.asciz '\r\n\r\nZ80 MEMBERSHIIP CARD FUZIX LOADER 0.1\r\n'
gogogo:
		.asciz '\r\nExecuting FUZIX...\r\n'
