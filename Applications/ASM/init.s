;
;	A simple testing setup for FUZIX
;
		.org 0x100
start:		jp	start2
		.db	'F'
		.db	'Z'
		.db	'X'
		.db	'1'
		.dw	0
start2:
		ld a, '*'
		out (1), a
		ld hl, 2
		push hl
		ld hl, devtty
		push hl
		ld hl, 1
		push hl
		rst 0x30
		ld hl, 13
		push hl
		ld hl, banner
		push hl
		ld hl, 0
		push hl
		ld hl, 8
		push hl
		rst 0x30
		ld hl, 0
		push hl
		ld hl, 37
		push hl
		rst 0x30
banner:
		.db 'Hello World'
		.db 13,10
devtty:		.db '/dev/tty1',0