; FUZIX boot sector for MSX2 using MegaFlashROM_SCC+_SD
;
	.module	boot
	.area	BOOT(ABS)
	.org	0xc000

	.macro	BDOS, func
	ld	c, func
	call	0xf37d
	.endm

	.macro	ENASLT, slotaddr, page
	ld	a, (slotaddr)
	ld	h, page
	call	0x24
	.endm

	.db	0xeb, 0xfe, 0x90; MSDOS boot signature
	.ascii	'FUZIX   '	; disk name
	.dw	512		; sector size (in bytes)
	.db	16		; cluster size (in sectors)
	.dw	1		; unused sectors
	.db	2		; FATs
	.dw	1		; directory entries (avoid garbage)
	.dw	2047		; disk size (in sectors)
	.db	0xf0		; media id
	.dw	3		; FAT size (in sectors)
	.dw	0		; sectors per track
	.dw	0		; sides
	.dw	0		; hidden sectors
	jr	boot		; MSXDOS2 boot signature
        .ascii	'VOL_ID'
        .db	0x00, 0x52, 0x34, 0xd6, 0xf8
        .ascii	'NEXTOR 2.0 FAT12   '
boot:	ret	nc
	ld	hl,#load
	ld	de,#0xc100
	ld	bc,#end-#load
	ldir
	jp	0xc100
load:	ld	sp, #0xf51f
	ENASLT	#0xf342, #0x40	; RAM in slot 1
	ld	de, #0x100
	BDOS	#0x1a		; set disk transfer address
	BDOS	#0x19		; get current drive
	ld	l, a
	ld	de, #1		; start of fuzix.com
	ld	h, #96		; size of fuzix.com
	BDOS	#0x2f		; absolute sector read
	jp	0x100
end:
