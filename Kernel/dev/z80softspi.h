#ifndef _Z80SOFTSPI_H
#define _Z80SOFTSPI_H
/*
 *	Interfaces provided by the Z80 SoftSPI that are not present
 *	in the core devsd code
 */

extern uint8_t spi_piostate;	/* values for unchanging bits */
extern uint16_t spi_port;	/* 16bit I/O port to use */
extern uint8_t spi_data;	/* bit to use for data out */
extern uint8_t spi_clock;	/* bit to use for clock out */
                                /* data in must be bit 7 */

/* SD card specific */

extern void sd_spi_rx_sector(uint16_t addr) __z88dk_fastcall;
extern void sd_spi_tx_sector(uint16_t addr) __z88dk_fastcall;

/* Aliases for any future split for systems with hardware SPI SD and
   other bitbang spi */

extern void spi_transmit_byte(uint8_t byte) __z88dk_fastcall;
extern uint8_t spi_receive_byte(void);

#endif
