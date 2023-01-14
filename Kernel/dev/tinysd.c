
/*
 *	A minimal SD implementation for tiny machines
 *	Assumes
 *	- Firmware initialized the device
 *	- Only supports primary partitions
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tinysd.h>
#include "printf.h"

/* Used by the asm helpers */
uint8_t sd_page;
uint8_t sd_raw;
uint_fast8_t sd_shift[MAX_SD] = { 0xFF, 0xFF };
uint32_t sd_lba[MAX_SD][MAX_PART + 1];

static uint_fast8_t sd_spi_wait(bool want_ff)
{
    unsigned int timer = set_timer_ms(500);
    uint_fast8_t b;

    do {
        b = sd_spi_receive_byte();
        if (want_ff) {
            if (b == 0xFF)
                break;
        }
        else if (b != 0xFF)
            break;
    } while(!timer_expired(timer));
    return b;
}

static int sd_send_command(uint_fast8_t cmd, uint32_t arg)
{
    uint8_t *p = &arg;
    uint_fast8_t n, res;

    sd_spi_raise_cs();
    sd_spi_receive_byte();
    sd_spi_lower_cs();
    if (sd_spi_wait(true) != 0xFF)
        return 0xFF;
    sd_spi_transmit_byte(cmd);
    p = ((uint8_t *)&arg)+3;
    sd_spi_transmit_byte(*(p--));                     /* Argument[31..24] */
    sd_spi_transmit_byte(*(p--));                     /* Argument[23..16] */
    sd_spi_transmit_byte(*(p--));                     /* Argument[15..8] */
    sd_spi_transmit_byte(*p);                         /* Argument[7..0] */
    sd_spi_transmit_byte(0x01);
    sd_spi_receive_byte();
    n = 20;
    do {
        res = sd_spi_receive_byte();
    } while ((res & 0x80) && --n);

    return res;         /* Return with the response value */
}

int sd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
	uint8_t dev = minor >> 4;
	uint16_t ct = 0;
	uint8_t *dptr;
	uint16_t nblock;
	uint32_t lba;

	sd_page = 0;
	sd_raw = rawflag;
	if (rawflag == 1) {
		if (d_blkoff(BLKSHIFT))
			return -1;
		sd_page = udata.u_page;		/* User space */
	} else if (rawflag == 2)
		sd_page = swappage;

	lba = udata.u_block;
	if (minor)
		lba += sd_lba[dev][minor];

	dptr = udata.u_dptr;
	nblock = udata.u_nblock;

	/* Here be dragons. In the swap case we will load over udata so watch
	   we avoid udata. values */
	while (ct < nblock) {
            if (sd_send_command(is_read ? CMD17 : CMD24,
                lba << sd_shift[dev])) {
                    kputs("E0");
                    goto error;
            }
	    if (is_read) {
        	if (sd_spi_wait(false) != 0xFE)
        	    goto error;
                sd_spi_receive_sector(dptr);
            } else {
	        if (sd_spi_wait(true) != 0xFF)
	            goto error;
                sd_spi_transmit_byte(0xFE);
                sd_spi_transmit_sector(dptr);
                sd_spi_transmit_byte(0xFF);
                sd_spi_transmit_byte(0xFF);
                if ((sd_spi_wait(false) & 0x1F) != 0x05)
                    goto error;
                while(sd_spi_receive_byte() == 0x00);
            }
            sd_spi_raise_cs();
            sd_spi_receive_byte();
            ct++;
            dptr += 512;
            lba++;
	}
	return ct << 9;
error:
	kprintf("sd: I/O error\n");
	udata.u_error = EIO;
	return -1;
}

int sd_open(uint8_t minor, uint16_t flag)
{
	uint8_t dev = minor >> 4;
	minor &= 0x0F;
	if (dev > MAX_SD || minor > MAX_PART || sd_shift[dev] == 0xFF) {
		udata.u_error = ENODEV;
		return -1;
	}
	return 0;
}

int sd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return sd_transfer(minor, true, rawflag);
}

int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	flag;
	return sd_transfer(minor, false, rawflag);
}

