;
;	Software SPI. Implements the minimal bits needed to support the
;	SD card interface. Does not implement simultaneous send/rx
;	currently. The hardware can obviously do it but nobody needs it
;	so far.
;
;	Alan Cox 2019 with some further optimizations by herrw@pluspora
;

	.module softsd

	.globl _sd_spi_tx_byte
	.globl _sd_spi_rx_byte
	.globl sd_spi_tx_sector
	.globl sd_spi_rx_sector
	.globl spi0_set_regs
	.globl spi0_bitbang_rx
	.globl spi0_bitbang_tx

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

	.globl _spi_tx_byte
	.globl _spi_rx_byte

	.area _COMMONMEM

; SDCC linker is too crap to handle this
;_sd_spi_tx_byte .equ _spi_tx_byte
;_sd_spi_rx_byte .equ _spi_receive_byte
_sd_spi_tx_byte:
	jp _spi_tx_byte
_sd_spi_rx_byte:
	jp _spi_rx_byte


sd_spi_rx_sector:
	exx
	call spi0_set_regs
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

sd_spi_tx_sector:
	exx
	call spi0_set_regs
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
