/*
 *	A minimal SD implementation for tiny machines
 *	Assumes
 *	- Firmware initialized the device
 *	- Only supports primary partitions
 *	- Uses tinydisk
 */

#include <kernel.h>
#include <kdata.h>
#include <timer.h>
#include <tinydisk.h>
#include <tinysd.h>

#ifdef CONFIG_TD_SD

/* Byte or block oriented */
uint_fast8_t sd_shift[CONFIG_TD_NUM];
/* Busy flag for bus sharing */
uint8_t tinysd_busy;
/* Current device (for simple setups always 0 and ignored) */
uint8_t tinysd_unit;

static uint_fast8_t sd_spi_wait(bool want_ff)
{
	unsigned int timer = set_timer_ms(500);
	uint_fast8_t b;

	do {
		b = sd_spi_rx_byte();
		if (want_ff) {
			if (b == 0xFF)
				break;
		} else if (b != 0xFF)
			break;
	} while (!timer_expired(timer));
	return b;
}

static int sd_send_command(uint_fast8_t cmd, uint32_t arg)
{
	uint8_t *p = (uint8_t *)&arg;
	uint_fast8_t n, res;

	sd_spi_raise_cs();
	sd_spi_rx_byte();
	sd_spi_lower_cs();
	if (sd_spi_wait(true) != 0xFF)
		return 0xFF;
	sd_spi_tx_byte(cmd);
#if defined(__SDCC) || defined(SDCC)
	p = ((uint8_t *) & arg) + 3;
	sd_spi_tx_byte(*(p--));	/* Argument[31..24] */
	sd_spi_tx_byte(*(p--));	/* Argument[23..16] */
	sd_spi_tx_byte(*(p--));	/* Argument[15..8] */
	sd_spi_tx_byte(*p);	/* Argument[7..0] */
#else
	sd_spi_tx_byte(arg >> 24);
	sd_spi_tx_byte(arg >> 16);
	sd_spi_tx_byte(arg >> 8);
	sd_spi_tx_byte(arg);
#endif
	sd_spi_tx_byte(0x01);
#ifndef CONFIG_TD_SD_EMUBUG        
	sd_spi_rx_byte();
#endif
	n = 20;
	do {
		res = sd_spi_rx_byte();
	} while ((res & 0x80) && --n);

	return res;		/* Return with the response value */
}

int sd_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t * dptr)
{
	uint32_t block = lba << sd_shift[dev];
	tinysd_busy = 1;
	tinysd_unit = dev;

	if (sd_send_command(is_read ? CMD17 : CMD24, block))
		goto error;
	if (is_read) {
		if (sd_spi_wait(false) != 0xFE)
			goto error;
		sd_spi_rx_sector(dptr);
	} else {
		if (sd_spi_wait(true) != 0xFF)
			goto error;
		sd_spi_tx_byte(0xFE);
		sd_spi_tx_sector(dptr);
		sd_spi_tx_byte(0xFF);
		sd_spi_tx_byte(0xFF);
		if ((sd_spi_wait(false) & 0x1F) != 0x05)
			goto error;
		while (sd_spi_rx_byte() == 0x00);
	}
	sd_spi_raise_cs();
	sd_spi_rx_byte();
	tinysd_busy = 0;
	return 1;
error:
	tinysd_busy = 0;
	return 0;
}
#endif
