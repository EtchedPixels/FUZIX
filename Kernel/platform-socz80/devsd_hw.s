; 2013-12-19 William R Sowerbutts

            .module devsd_hw

            ; exported symbols
            .globl _sd_spi_clock
            .globl _sd_spi_raise_cs
            .globl _sd_spi_lower_cs
            .globl _sd_spi_receive_byte
            .globl _sd_spi_transmit_byte
            .globl _sd_spi_receive_to_memory
            .globl _sd_spi_transmit_from_memory
	    .globl _sd_spi_mode0

            .include "socz80.def"

            .area _CODE

_sd_spi_clock:
            pop de
            pop hl
            push hl
            push de
            ld a, l
            out (SD_SPI_DIVISOR), a
            ret

_sd_spi_raise_cs:
            ld a, #0xFF
            out (SD_SPI_CHIPSELECT), a
            ret

_sd_spi_lower_cs:
            ld a, #0xFE
            out (SD_SPI_CHIPSELECT), a
            ret

_sd_spi_mode0:
	   xor a
           out (SD_SPI_MODE), a
           ret

_sd_spi_receive_byte:
            ; read a byte
            ld a, #0xFF
            out (SD_SPI_TX), a
            in a, (SD_SPI_RX)
            ld h, #0
            ld l, a
            ret

_sd_spi_transmit_from_memory:
            pop de  ; return address
            pop hl  ; memory pointer
            pop bc  ; byte count
            push bc ; now put the stack back ...
            push hl
            push de
tnextbyte:
            ld a, (hl)
            out (SD_SPI_TX), a
            inc hl
            dec bc
            ld a, b
            or c
            jr nz, tnextbyte
            ; return success in HL
            ld hl, #1
            ret


_sd_spi_receive_to_memory:
            pop de  ; return address
            pop hl  ; memory pointer
            pop bc  ; byte count
            push bc ; now put the stack back ...
            push hl
            push de
rnextbyte:
            ld a, #0xFF
            out (SD_SPI_TX), a
            in a, (SD_SPI_RX)
            ld (hl), a
            inc hl
            dec bc
            ld a, b
            or c
            jr nz, rnextbyte
            ; there's also a 16-bit CRC that we discard
            call _sd_spi_receive_byte
            call _sd_spi_receive_byte
            ; return success in HL
            ld hl, #1
            ret

_sd_spi_transmit_byte:
            pop de
            pop hl
            push hl
            push de
            ld a, l
            out (SD_SPI_TX), a
            ret
