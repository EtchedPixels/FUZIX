#include <kernel.h>
#include <i2c.h>
#include <platform_i2c.h>
#include <i2c_bitbang.h>

#ifdef CONFIG_I2C_BITBANG
/*
 *	Generic bitbang I2C support layer
 *
 *	TODO: support clock stretching timeout
 */

static void i2c_clock_up(uint_fast8_t bus)
{
    i2c_set(bus, I2C_SCL);
    /* FIXME: timeouts */
    while(!i2c_scl(bus));
}

/* Signal start and hold the bus */
static void i2c_start(uint_fast8_t bus)
{
    i2c_set(bus, I2C_SCL);
    i2c_set(bus, I2C_SDA);
    i2c_clear(bus, I2C_SDA);
    i2c_clear(bus, I2C_SCL);
}

/* Signal stop and release the bus */
static void i2c_stop(uint_fast8_t bus)
{
    i2c_clear(bus, I2C_SDA);
    i2c_clear(bus, I2C_SCL);
    i2c_set(bus, I2C_SCL);		/* Tristate bus */
    i2c_set(bus, I2C_SDA);
}

/* Read a byte from the bus - the bus must be held at this point */
static uint_fast8_t i2c_read(uint_fast8_t bus)
{
    uint_fast8_t n = 8;
    uint_fast8_t v = 0;

    i2c_set(bus, I2C_SDA);		/* Data tristate */
    while(n--) {
        i2c_clock_up(bus);
        v <<= 1;
        if (i2c_sda(bus))
            v |= 1;
        i2c_clear(bus, I2C_SCL);
    }
    i2c_clear(bus, I2C_SDA);
    i2c_clock_up(bus);
    i2c_clear(bus, I2C_SCL);
    return v;
}

/* Write a byte to the bus - the bus must be held at this point */
static uint_fast8_t i2c_write(uint_fast8_t bus, uint_fast8_t val)
{
    uint_fast8_t n = 8;
    uint_fast8_t v;
    while(n--) {
        if (val & 0x80)
            i2c_set(bus, I2C_SDA);
        else
            i2c_clear(bus, I2C_SDA);
        v <<= 1;
        i2c_clock_up(bus);
        i2c_clear(bus, I2C_SCL);
    }
    /* Now check for an ACK */
    /* Tristate the bus */
    i2c_clock_up(bus);
    i2c_set(bus, I2C_SCL);
    /* The device should pull SDA down */
    v = i2c_sda(bus);
    /* Now take the bus back */
    i2c_clear(bus, I2C_SCL);
    return v;
}

/*
 *	I2C interface layer
 *
 *	We don't currently provide any support for 10bit or for weird stuff
 *	like restarts mid flow.
 */

int i2c_bb_receive(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len)
{
    unsigned int received = 0;

    if (i2c_claim_bus(bus))
        return -1;

    i2c_start(bus);

    if (i2c_write(bus, dev)) {
        /* Nobody home */
        i2c_stop(bus);
        return -ENODEV;
    }

    while(len--) {
        if (i2c_write(bus, *buf++))
            break;
        received++;
    }

    i2c_stop(bus);
    i2c_release_bus(bus);
    return received;
}

int i2c_bb_send(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len)
{
    unsigned int sent = 0;

    if (i2c_claim_bus(bus))
        return -1;

    i2c_start(bus);

    if (i2c_write(bus, dev)) {
        /* Nobody home */
        i2c_stop(bus);
        i2c_release_bus(bus);
        return -ENODEV;
    }

    while(len--) {
        if (i2c_write(bus, *buf++))
            break;
        sent++;
    }

    i2c_stop(bus);
    i2c_release_bus(bus);
    return sent;
}




#endif
