;
;	Software SPI. Implements the minimal bits needed to support the
;	SD card interface
;
;	Alan Cox 2019 with some further optimizations by herrw@plusporta
;

	.module softspi

	.globl _sd_spi_transmit_byte
	.globl _sd_spi_receive_byte
	.globl _sd_spi_tx_sector
	.globl _sd_spi_rx_sector

        .include "kernel.def"
        .include "../kernel.def"


	.globl _spi_port		; Port to use
	.globl _spi_piostate		; PIO bits to preserve

	.area _COMMONMEM

	; Must be in common space

_spi_port:
	.dw	0
_spi_piostate:
	.db	0

sd_spi_set_regs:
	ld a,(_spi_piostate)
	ld e,a
	ld a,#SPI_DATA
	or e
	ld l,a
	or #SPI_CLOCK
	ld h,a
	ld a,e
	or #SPI_CLOCK
	ld d,a
	ld bc, (_spi_port)
	ret	

_sd_spi_transmit_byte:
	ld a,l				; C argument
	push af
	call sd_spi_set_regs
	pop af
	call spi0_bitbang_tx
	out (c),e			; drop final clock
	ret

_sd_spi_receive_byte:
	call sd_spi_set_regs
	call spi0_bitbang_rx
	out (c),e
	ld l,d
	ret

_sd_spi_rx_sector:
	exx
	call sd_spi_set_regs
	exx
	ld b,#0
spi_rx_loop:
	exx
	call spi0_bitbang_rx
	ld a,d
	exx
	ld (hl),a
	inc hl
	exx
	call spi0_bitbang_rx
	ld a,d
	exx
	ld (hl),a
	inc hl
	djnz spi_rx_loop
	exx
	out (c),e
	exx
	ret

_sd_spi_tx_sector:
	exx
	call sd_spi_set_regs
	exx
	ld b,#0
spi_tx_loop:
	ld a,(hl)
	inc hl
	exx
	call spi0_bitbang_tx
	exx
	ld a,(hl)
	inc hl
	exx
	call spi0_bitbang_tx
	exx
	djnz spi_tx_loop
	exx
	out (c),e
	exx
	ret

;
;	The low level bits for the Z80 PIO
;
;	The basic idea is that we keep the port in C and each needed byte
;	value in a register. The values are all precomputed so we don't
;	touch other bits and so we can use different modes
;
;	We could unroll tx but it's bigger than rx and we actually don't
;	do that many writes compared to reads.
;	
spi0_bitbang_tx:
	ld b,#8			; 7
spi0_bit_tx:
	rla			; 4
	jp nc, spi0_tx0		; 10		For SPI 0
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1	(sample)
	djnz spi0_bit_tx	; 13/8
	ret			; 10
spi0_tx0:
	out (c),e		; 12		low | 0
	out (c),d		; 12		high | 1	(sample)
	djnz spi0_bit_tx	; 13/8
	ret
;
;	Send 0xFF and receive a byte.
;
;	With call overhead about 500 clocks a byte or about 14Kbytes/second
;	at 7.3MHz with a djnz loop
;
;	Unrolled/tweaked by herrw@pluspora to get us down to 382 clocks a
;	byte, or about 19K/second !
;
spi0_bitbang_rx:
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in d,(c)		; 12		get bit 0
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		and bit 1
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		and bit 2
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		3
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		4
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		5
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		6
	rra			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		7
	rra			; 4
	rl d			; 8
	ret
