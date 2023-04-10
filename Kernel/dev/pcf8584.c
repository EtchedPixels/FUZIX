/*
 *	PCF8584 Interface: master mode only
 */

#include <kernel.h>
#include <printf.h>
#include <i2c.h>
#include <pcf8584.h>

/*  __sfr __at 0xFE	i2c_s0;
    __sfr __at 0xFF i2c_s1; */

#define BUS_CLEAR		0xC1
#define SEND_STOP		0xC2
#define SEND_STOP_ACK		0xC3
#define SEND_START		0xC4
#define SEND_START_ACK		0xC5
#define NACK			0x40

#define BUS_BUSY		0x01
#define BUS_PIN			0x80
#define BUS_LRB			0x08

static void nap(void)
{
    /* This is excessive but helps cover the crazies with 36MHz Z180 boxes */
    volatile uint8_t i;
    for (i = 0; i < 200; i++);
}
    
void pcf8584_init(uint8_t clock)
{
    i2c_s1 = 0x80;		/* PIN is reset when written */
    i2c_s0 = 0x55;		/* Our address 0xAA (shifted) */
    i2c_s1 = 0xA0;		/* PIN | ES1 - select alt reg 1 */
    i2c_s0 = clock;		/* Set clock - about 90KHz */
    i2c_s1 = BUS_CLEAR;
    nap();
}

void pcf8584_bus_reset(void)
{
    i2c_s1 = SEND_STOP;
    nap();
    i2c_s1 = BUS_CLEAR;
}

/* For multiple master cases */
static int pcf8584_bus_wait(void)
{
    unsigned timeout = 200;
    while((i2c_s1 & BUS_BUSY) == 0) {
        if (!(--timeout))
            return -EBUSY; 
        nap();
    }
    return 0;
}

static int pcf8584_pin_wait(void)
{
    unsigned timeout = 200;
    while((i2c_s1 & BUS_PIN)) {
        if (!(--timeout))
            return -ETIMEDOUT;
        nap();
    }
    return 0;
}

static int pcf8584_ack_pin(void)
{
    int err = pcf8584_pin_wait();
    if (err)
        return err;
    if (i2c_s1 & BUS_LRB) 
        return -1;
    return 0;
}

static int pcf8584_connect(uint8_t addr)
{
    int r;
    i2c_s0 = addr;
    i2c_s1 = SEND_START_ACK;
    return pcf8584_ack_pin();
}

static int pcf8584_xmit(uint8_t byte)
{
    i2c_s0 = byte;
    return pcf8584_pin_wait();
}

static int pcf8584_recv(void)
{
    int err;
    err = pcf8584_pin_wait();
    if (err)
        return err;
    return i2c_s0;
}

int pcf8584_rx_msg(uint8_t addr, uint8_t *buf, unsigned len)
{
    int r;
    r = pcf8584_bus_wait();
    if (r)
        return r;
    r = pcf8584_connect(addr);
    if (r)
        goto fail;
    i2c_s0;		/* Start receiving */
    while(len > 1) {
        r = pcf8584_recv();
        if (r < 0)
            goto fail;
        if (i2c_s1 & BUS_LRB) {
            r = -EIO;
            goto fail;
        }
        *buf++ = r;
        len--;
    }
    /* Final byte */
    i2c_s1 = NACK;
    r = pcf8584_recv();
    if (r == -1)
        return -1;
    *buf = r;
    i2c_s1 = SEND_STOP_ACK;
    return 0;
fail:
    i2c_s1 = SEND_STOP;
    return r;
}

int pcf8584_tx_msg(uint8_t addr, const uint8_t *buf, unsigned len)
{
    int err = pcf8584_bus_wait();
    if (err)
        return err;
    err = pcf8584_connect(addr);
    if (err)
        goto fail;
    while(len) {
        err = pcf8584_xmit(*buf++);
        if (err)
            goto fail;
        len--;
    }
    i2c_s1 = SEND_STOP_ACK;
    return 0;
fail:
    i2c_s1 = SEND_STOP;
    return err;
}

