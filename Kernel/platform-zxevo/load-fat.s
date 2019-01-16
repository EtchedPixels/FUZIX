;
;	Load the kernel from sectors 3+ of the media
;

	.area _BOOT(ABS)

	.org 0x5FEF		; 17 lead bytes

header:
	.db 'F','U','Z','I','X', 0, 0, 0		; name
	.db 'C'			; code type
	.db 0x00, 0x60		; load
	.dw 0x0200		; size
	.dw 0x0002		; in sectors
	.dw 0x3FA6		; checksum

	; Code at 0x6000 as with a direct HD load

	.include "load-hd.s"
