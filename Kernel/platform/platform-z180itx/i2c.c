/*
 *	I2C interface glue (TODO)
 */

#include <kernel.h>
#include <kdata.h>
#include <z180itx.h>
#include <ds1302.h>
#include <i2c.h>
#include <i2c_bitbang.h>

uint8_t rtc_defer;
__sfr __at 0x40 ppi_a;
__sfr __at 0x41 ppi_b;

/* There isn't a huge point optimizing this, we are not supposed to go over
   100 or 400KHz anyway. The expectations of the caller are that
   - any delays needed are made before the signal is changed
   - on a read a delay is made before the signal is read
   
   But basically it only matters that both are the same side. */

void i2c_set(uint_fast8_t bus, uint_fast8_t val)
{
	ppi_a |= val;
}

void i2c_clear(uint_fast8_t bus, uint_fast8_t val)
{
	ppi_a &= ~val;
}

uint_fast8_t i2c_sda(uint_fast8_t bus)
{
	return ppi_b & I2C_RX_SDA;
}

/* The SC126 cannot handle clock stretching */
uint_fast8_t i2c_scl(uint_fast8_t bus)
{
	return ppi_b & I2C_RX_SCL;
}

int i2c_claim_bus(uint_fast8_t bus)
{
	if (bus) {
		udata.u_error = ENODEV;
		return -1;
	}
	/* Don't poll the RTC whilst we are busy with the port */
	rtc_defer = 1;
	return 0;
}

void i2c_release_bus(uint_fast8_t bus)
{
	used(bus);
	rtc_defer = 0;
}

int plt_i2c_msg(struct i2c_msg *m, uint8_t *kbuf)
{
    if (m->bus)
        return -ENODEV;
    if (m->addr & 1)
        i2c_bb_receive(m->bus, m->addr, kbuf, m->len);
    else
        i2c_bb_send(m->bus, m->addr, kbuf, m->len);
}
