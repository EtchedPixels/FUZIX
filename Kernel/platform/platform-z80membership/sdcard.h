/*
 *	Interfaces provided by the Z80 SoftSPI that are not present
 *	in the core devsd code
 */

extern void sd_spi_rx_sector(uint16_t addr) __z88dk_fastcall;
extern void sd_spi_tx_sector(uint16_t addr) __z88dk_fastcall;

