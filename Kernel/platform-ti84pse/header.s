	; 0x0000
	; XXX: This should be a separate module, probably
_rst00:
	; TODO: This should jump to #null_handler post-boot
	jp init
	.ds 5
_rst08:
	.ds 8
_rst10:
	.ds 8
_rst18:
	.ds 8
_rst20:
	.ds 8
_rst28:
	.ds 8 ; XXX: panic kernel here for unknown restart?
_rst30:
	.ds 8 ; TODO: syscall entry
_rst38:
	; Mode 1 interrupt handler
	.ds 8

	.ds 19
_boot:
	; ASIC jumps here
	jp init
bootmagic:
	.db 0xFF, 0xA5, 0xFF
