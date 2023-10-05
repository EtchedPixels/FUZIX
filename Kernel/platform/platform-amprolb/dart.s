;=========================================================================
;
;	Logic for IM2 interrupt driven DART
;
;=========================================================================

	.globl _sio_state
	.globl _sio_dropdcd
	.globl _sio_rxl
	.globl _sio_txl

	.globl	siob_txd
	.globl	siob_status
	.globl	siob_rx_ring
	.globl	siob_special
	.globl	sioa_txd
	.globl	sioa_status
	.globl	sioa_rx_ring
	.globl	sioa_special

	.include "kernel.def"

	.area _SERIALDATA

; These are laid out and exposed as arrays to C
_sio_wr5:
_sioa_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_siob_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_sio_flow:
_sioa_flow:
	.db 0			; Flow starts off
_siob_flow:
	.db 0			; Flow starts off
_sio_state:
_sioa_state:
	.db 0			; Last status report
_siob_state:
	.db 0			; Last status report
_sio_dropdcd:
_sioa_dropdcd:
	.db 0			; DCD dropped since last checked
_siob_dropdcd:
	.db 0			; DCD dropped since last checked
_sio_rxl:
_sioa_rxl:
	.db 0
_siob_rxl:
	.db 0
_sio_txl:
_sioa_txl:
	.db 0
_siob_txl:
	.db 0

	.include "../dev/z80sio.s"

sio_ports a
sio_ports b

	.area _COMMONMEM

;
;	No magic needed as we keep our buffers in common
;
.macro switch
.endm

.macro switchback
.endm

sio_handler_im2	a, DARTA_C, DARTA_D, reti
sio_handler_im2 b, DARTB_C, DARTB_D, reti
