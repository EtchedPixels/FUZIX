;
;	Load our kernel image via TRDOS
;
;	The disk holds files of the format code{n} where n is the bank to
;	place it in. TRDOS is asked to load the lot and then we launch
;
;	The on disk layout is
;
;	16K common	(0x4000-0x7FFF)		Bank 5 (loaded via 4)
;	16K bank	(0xC000-0xFFFF)		Bank 0	(CODE1)
;	16K common	(0x8000-0xBFFF)		Bank 2
;	16K bank	(0xC000-0xFFFF)		Bank 1	(CODE2)
;	16K bank	(0xC000-0xFFFF)		Bank 7  (CODE3)
;	16K bank	(0xC000-0xFFFF)		Bank 3  (CODE4)
;
;

		.module loader
		.area CODE(ABS)
		.org 0x7800

start:
		ld sp,#start

		ld a,#4			; common 4000-7FFF temporary copy
		ld de,#0x0109
		call trdos

		xor a			; code 1 into bank 0
		ld de,#0x0509
		call trdos

		ld a,#2			; common 8000-BFFF into bank 2
		ld de,#0x0909
		call trdos

		ld a,#1			; code 2 into bank 1
		ld de,#0x0D09
		call trdos

		ld a,#7			; code  3 into bank 7
		ld de,#0x1109
		call trdos

		ld a,#3			; code 4 into bank 3
		ld de,#0x1509
		call trdos

		di

		jp 0x8400		; into loaded bootstrap

trdos:
		; Switch bank
		push af
		ld bc,#0x7ffd
		out (254),a
		or #0x10
		out (c),a
		ld hl,#0xC000
		ld bc,#0x4005
		call 15635
		pop af
		ret
