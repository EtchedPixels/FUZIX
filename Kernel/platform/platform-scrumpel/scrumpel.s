;
;	Scrumpel support code. Based upon code by Will Sowerbutts
;
        .module sc111
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl inchar
        .globl outchar
        .globl plt_interrupt_all
        .globl _bufpool

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl outhl
        .globl outnewline

        .include "kernel.def"
        .include "../../cpu-z180/z180.def"
        .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; Buffers
; -----------------------------------------------------------------------------
        .area _BUFFERS
_bufpool:
        .ds (BUFSIZE * 5) ; adjust NBUFS in config.h in line with this

; -----------------------------------------------------------------------------
; Initialisation code
; -----------------------------------------------------------------------------
        .area _DISCARD

init_early:
	ret

init_hardware:
        ; enable ASCI interrupts
        in0 a, (ASCI_STAT0)
        or #0x08                ; enable ASCI0 receive interrupts
        out0 (ASCI_STAT0), a
        in0 a, (ASCI_ASEXT0)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT0), a
        in0 a, (ASCI_STAT1)
        or #0x08                ; enable ASCI1 receive interrupts
        out0 (ASCI_STAT1), a
        in0 a, (ASCI_ASEXT1)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT1), a

        jp z180_init_hardware

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------
        .area _COMMONMEM

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
        push bc
        ld b, a
        ; wait for transmitter to be idle
ocloop: in0 a, (ASCI_STAT0)
        bit 1, a        ; and 0x02
        jr z, ocloop    ; loop while busy
        ; now output the char to serial port
        ld a, b
        out0 (ASCI_TDR0), a
        pop bc
        ret

; inchar: Wait for character on UART, return in A
; destroys: AF
inchar:
        in0 a, (ASCI_STAT0)
        rlca
        jr nc, inchar
        in0 a, (ASCI_RDR0)
        ret

plt_interrupt_all:
        ret


;
;	Low level SPI bitbanging
;
;	Based on the Z80 PIO interface. Assumes that the SPI interface
;	is wired to an SD card through the relevant voltage shifters and
;	pullup.
;

	.globl _sd_spi_transmit_byte
	; Send the byte in L, received data is ignored
	.globl _sd_spi_receive_byte
	; Receive a byte in L, send FF
	; Optimized block transfer routines
	.globl _sd_spi_tx_sector
	.globl _sd_spi_rx_sector

	; Other port bit status
	.globl _spi_bits

SPI_PORT	.equ	0xA0

SPI_DATA	.equ	1
SPI_CLOCK	.equ	2

;
;	The basic idea is that we set up de and hl to bit bang a 0 or 1
;	c to point to the port so we can use out (c), and b is free
;	for djnz.
;
sd_spi_set_regs:
	ld a,(_spi_bits)
	ld e,a
	ld a,#SPI_DATA
	or e
	ld l,a
	or #SPI_CLOCK
	ld h,a
	ld a,e
	or #SPI_CLOCK
	ld d,a
	ld bc, #SPI_PORT
	ret	

_sd_spi_transmit_byte:
	ld a,l
	push af
	call sd_spi_set_regs
	pop af
	call spi0_bitbang_tx
	out (c),e
	ret

_sd_spi_receive_byte:
	call sd_spi_set_regs
	call spi0_bitbang_rx
	out(c),e
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
;	Now the scary stuff
;

spi0_bitbang_tx:
	ld b,#8
spi0_bit_tx:
	rla
	jp nc, spi0_tx0			; sending 0 or 1 ?
	out (c),l
	out (c),h
	djnz spi0_bit_tx
	ret
spi0_tx0:
	out (c),e
	out (c),d
	djnz spi0_bit_tx
	ret

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
