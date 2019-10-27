#include <kernel.h>
#include <kdata.h>
#include <i2c.h>

/*
 *	I2C interface layer
 *
 *	We don't currently provide any support for 10bit or for weird stuff
 *	like restarts mid flow.
 */

int i2c_receive(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len)
{
    unsigned int received = 0;

    if (i2c_claim_bus(bus))
        return -1;

    i2c_start(bus);

    if (i2c_write(bus, dev | I2C_READ)) {
        /* Nobody home */
        i2c_stop(bus);
        udata.u_error = ENODEV;
        return -1;
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

int i2c_send(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len)
{
    unsigned int sent = 0;

    if (i2c_claim_bus(bus))
        return -1;

    i2c_start(bus);

    if (i2c_write(bus, dev | I2C_WRITE)) {
        /* Nobody home */
        i2c_stop(bus);
        udata.u_error = ENODEV;
        return -1;
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

/* User space interface */
#ifdef CONFIG_DEV_I2C

int devi2c_write(void)
{
    uint8_t *t;
    int r = -1;
    if (udata.u_count > BLKSIZE || udata.u_count < 3) {
        udata.u_error = EMSGSIZE;
        return -1;
    }
    t = tmpbuf();
    if (!uget(udata.u_base, t, udata.u_count))
        r = i2c_send(*t, t[1], t + 2, udata.u_count - 2);
    tmpfree(t);
    return r;
}

int devi2c_read(void)
{
    uint8_t *t;
    int r = -1;
    if (udata.u_count > BLKSIZE || udata.u_count < 3) {
        udata.u_error = EMSGSIZE;
        return -1;
    }
    t = tmpbuf();
    if (!uget(udata.u_base, t, 2)) {
        r = i2c_receive(*t, t[1], t + 2, udata.u_count - 2);
        if (r > 0)
            if (uput(t, udata.u_base + 2, r))
                r = -1;
    }
bad:
    tmpfree(t);
    return r;
}

#endif
