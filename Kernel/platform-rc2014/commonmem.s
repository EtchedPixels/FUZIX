;
;	Common is placed at 0xF000 by fuzix.lnk
;

        .module commonmem

        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"

;
;	Page aligned here so use it for the zip drive table
;
;	Keep at least 32 byte aligned as the z180 int table
;	follows next
;

	.globl lpxlate4

;
;	Convert between MG014 bit ordering of the status
;	bits and nybbles on the PPA. We end up with a table
;	because the PPA is designed for IBM order.
;
lpxlate4:
	; Lower nybble
	.byte 0x08  ; 00
	.byte 0x0C  ; 01
	.byte 0x00  ; 02
	.byte 0x04  ; 03
	.byte 0x0A  ; 04
	.byte 0x0E  ; 05
	.byte 0x02  ; 06
	.byte 0x06  ; 07
	.byte 0x09  ; 08
	.byte 0x0D  ; 09
	.byte 0x01  ; 0A
	.byte 0x05  ; 0B
	.byte 0x0B  ; 0C
	.byte 0x0F  ; 0D
	.byte 0x03  ; 0E
	.byte 0x07  ; 0F
	; Upper nybble
	.byte 0x80  ; 00
	.byte 0xC0  ; 01
	.byte 0x00  ; 02
	.byte 0x40  ; 03
	.byte 0xA0  ; 04
	.byte 0xE0  ; 05
	.byte 0x20  ; 06
	.byte 0x60  ; 07
	.byte 0x90  ; 08
	.byte 0xD0  ; 09
	.byte 0x10  ; 0A
	.byte 0x50  ; 0B
	.byte 0xB0  ; 0C
	.byte 0xF0  ; 0D
	.byte 0x30  ; 0E
	.byte 0x70  ; 0F
