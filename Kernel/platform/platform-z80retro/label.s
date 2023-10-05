;
;	Label so we can boot from the ROM
;
	.byte	0x01
	.byte	0x01
	.ascii  "FUZIX   "
	.byte	0	; 32bit offset
	.byte	0x80	; Block 64
	.byte	0
	.byte	0
	.word	0xFE00
	.word	0x0200
	.word	0xFE00
	.byte	0
