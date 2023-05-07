/*
 *	SC126 I2C interface support
 *
 *	Conflicts with RTC so RTC reading needs to be deferred when doing
 *	interrupt/rtc syncs
 */

#include <kernel.h>
#include <kdata.h>
#include <rcbus-z180.h>
#include <ds1302.h>
#include <i2c.h>
#include <i2c_bitbang.h>

__sfr __at 0x0C i2c;

/* There isn't a huge point optimizing this, we are not supposed to go over
   100 or 400KHz anyway. The expectations of the caller are that
   - any delays needed are made before the signal is changed
   - on a read a delay is made before the signal is read
   
   But basically it only matters that both are the same side. */

void i2c_set(uint_fast8_t bus, uint_fast8_t val)
{
    used(bus);
    gpio_set(val, val);
}

void i2c_clear(uint_fast8_t bus, uint_fast8_t val)
{
    used(bus);
    gpio_set(val, 0);
}

uint_fast8_t i2c_sda(uint_fast8_t bus)
{
    used(bus);
    return i2c & I2C_RX_SDA;
}

/* The SC126 cannot handle clock stretching */
uint_fast8_t i2c_scl(uint_fast8_t bus)
{
    used(bus);
    return I2C_RX_SCL;
}

int i2c_claim_bus(uint_fast8_t bus)
{
    if (bus) {
        udata.u_error = ENODEV;
        return -1;
    }
    /* Don't poll the RTC whilst we are busy with the port */
#ifdef CONFIG_RTC_DS1302
    rtc_defer = 1;
#endif
    return 0;
}

void i2c_release_bus(uint_fast8_t bus)
{
    used(bus);
#ifdef CONFIG_RTC_DS1302
    rtc_defer = 0;
#endif
}

int plt_i2c_msg(struct i2c_msg *m, uint8_t *kbuf)
{
    if (m->bus)
        return -ENODEV;
    if (m->addr & 1)
        return i2c_bb_receive(m->bus, m->addr, kbuf, m->len);
    else
        return i2c_bb_send(m->bus, m->addr, kbuf, m->len);
}
