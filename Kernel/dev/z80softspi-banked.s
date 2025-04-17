;
;	Software SPI. Implements the minimal bits needed to support the
;	SD card layer (see z80sftsd.s) and other spi users
;
;	Alan Cox 2019 with some further optimizations by herrw@pluspora
;

	.module softspi

	.globl _spi_tx_byte
	.globl _spi_rx_byte
	.globl spi0_set_regs
	.globl spi0_bitbang_rx
	.globl spi0_bitbang_tx

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"


	.globl _spi_port		; Port to use
	.globl _spi_piostate		; PIO bits to preserve
	.globl _spi_data		; Output data bit
	.globl _spi_clock		; Clock bit

	.area _COMMONMEM

	; Must be in common space

_spi_port:
	.dw	0
_spi_piostate:
	.db	0

_spi_data:
	.db	0
_spi_clock:
	.db	0

spi0_set_regs:
	ld bc,(_spi_data)		; C = SPI_DATA, B = SPI_CLOCK
	ld a,(_spi_piostate)
	ld e,a
	ld a,c		; data
	or e
	ld l,a
	or b		; clock
	ld h,a
	ld a,e
	or b		; clock
	ld d,a
	ld bc, (_spi_port)
	ret	

_spi_tx_byte:
	pop bc
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a,l				; C argument
	push af
	call spi0_set_regs
	pop af
	call spi0_bitbang_tx
	out (c),e			; drop final clock
	ret

_spi_rx_byte:
	call spi0_set_regs
	call spi0_bitbang_rx
	out (c),e
	ld l,d
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
spi0_bit0_tx:
	rla			; 4
	jr nc, spi0_bit0_tx0	; 10		For SPI 0
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1	(sample)
	jr spi0_bit1_tx		; 10
spi0_bit0_tx0:
	out (c),e		; 12		low | 0
	out (c),d		; 12		high | 0	(sample)
spi0_bit1_tx:
	rla
	jr nc, spi0_bit1_tx0
	out (c),l
	out (c),h
	jr spi0_bit2_tx
spi0_bit1_tx0:
	out (c),e
	out (c),d
spi0_bit2_tx:
	rla
	jr nc, spi0_bit2_tx0
	out (c),l
	out (c),h
	jr spi0_bit3_tx
spi0_bit2_tx0:
	out (c),e
	out (c),d
spi0_bit3_tx:
	rla
	jr nc, spi0_bit3_tx0
	out (c),l
	out (c),h
	jr spi0_bit4_tx
spi0_bit3_tx0:
	out (c),e
	out (c),d
spi0_bit4_tx:
	rla
	jr nc, spi0_bit4_tx0
	out (c),l
	out (c),h
	jr spi0_bit5_tx
spi0_bit4_tx0:
	out (c),e
	out (c),d
spi0_bit5_tx:
	rla
	jr nc, spi0_bit5_tx0
	out (c),l
	out (c),h
	jr spi0_bit6_tx
spi0_bit5_tx0:
	out (c),e
	out (c),d
spi0_bit6_tx:
	rla
	jr nc, spi0_bit6_tx0
	out (c),l
	out (c),h
	jr spi0_bit7_tx
spi0_bit6_tx0:
	out (c),e
	out (c),d
spi0_bit7_tx:
	rla
	jr nc, spi0_bit7_tx0
	out (c),l
	out (c),h
	ret
spi0_bit7_tx0:
	out (c),e
	out (c),d
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
	LOADFIRST
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		1
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		2
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		3
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		4
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		5
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		6
	ROTATE			; 4
	rl d			; 8
	out (c),l		; 12		low | 1
	out (c),h		; 12		high | 1
	in a,(c)		; 12		7
	ROTATE			; 4
	rl d			; 8
	ret
