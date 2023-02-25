;
;	Dual SIO serial driver. For latency reasons this all
;	lives in the fixed bank space
;

	.module serial

        .area _SERIALDATA

	.include "kernel.def"

	.globl _sio_state
	.globl _sio_dropdcd
	.globl _sio_rxl
	.globl _sio_txl

	.globl _vectors

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

;
;	This macro builds the data area
;
sio_ports a
sio_ports b
;
;	And this one writes the code
;
        .area	_COMMONMEM
;
;	No bank switching needed on entry/exit
;
.macro switch
.endm

.macro switchback
.endm

sio_handler_im2 a, SIOA_C, SIOA_D, reti
sio_handler_im2 b, SIOB_C, SIOB_D, reti

;
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld	b,#0x06
	ld	c,l
	ld	hl,#_sio_r
	otir
	ret

outchar:
	push	af
	; wait for transmitter to be idle
ocloop_sio:
        xor	a                   ; read register 0
        out	(SIOA_C), a
	in	a,(SIOA_C)		; read Line Status Register
	and	#0x04			; get THRE bit
	jr	z,ocloop_sio
	; now output the char to serial port
	pop	af
	out	(SIOA_D),a
	ret

        .area _DISCARD

        .globl	sio_install
	.globl outchar

sio_install:
	ld	hl,#sio_setup
	ld	bc,#0x0A00 + SIOA_C	; 10 bytes to SIOA_C
	otir
	ld	hl,#sio_setup
	ld	bc,#0x0C00 + SIOB_C	; and 12 to SIOB_C (vector)
	otir

        ld	hl,#siovec
        ld	de,#_vectors+0x10
        ld	bc,#16
        ldir
        ret

siovec:
        .word	siob_txd
        .word	siob_status
        .word	siob_rx_ring
        .word	siob_special
        .word	sioa_txd
        .word	sioa_status
        .word	sioa_rx_ring
        .word	sioa_special

RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

sio_setup:
	.byte	0x00
	.byte	0x18		; Reset
	.byte	0x04
	.byte	0xC4
	.byte	0x01
	.byte	0x1F
	.byte	0x03
	.byte	0xC1
	.byte	0x05
	.byte	RTS_LOW
	.byte	0x02
	.byte	0x10		; vector 0x10 for serial (0x00 for CTC)

