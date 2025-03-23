/*
 *	IO processor related bits
 */

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <input.h>
#include <tinydisk.h>
#include <rtc.h>
#include <iop.h>

#define NR_DISK		4

static int iop_fd[NR_DISK];
static uint32_t iop_lba[NR_DISK];

void iop_data(uint_fast8_t x)
{
    while(in(0xF4) & 2);
    out(0xF5, x);
}

void iop_cmd(uint_fast8_t x)
{
    /* Empty FIFO (shouldn't be needed) */
    while(in(0xF4) & 1)
        in(0xF5);
    /* Send start of command and cmd byte */
    out(0xF4, 0x80);
    iop_data(x);
}

uint_fast8_t iop_rx(void)
{
    while((in(0xF4) & 1) == 0);
    return in(0xF5);
}

void iop_write(uint8_t *p, unsigned len)
{
    while(len--)
        iop_data(*p++);
}

void iop_read(uint8_t *p, unsigned len)
{
    while(len--)
        *p++ = iop_rx();
}

static int open_disk(uint_fast8_t minor)
{
    iop_cmd(0x10);
    iop_data(0x03);
    iop_write("fuzix", 5);
    iop_data('0' + minor);
    iop_data(0);
    return iop_rx();
}

static int iop_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    int c;
    int fd = iop_fd[dev];
    lba <<= 9;

    if (lba != iop_lba[dev]) {
        iop_cmd(0x14);
        iop_cmd(fd);
        iop_write((uint8_t *)&lba, 4);
        if (c = iop_rx()) {
bad:
            kprintf("hd%u: error %u\n", dev, -c);
            /* Force a seek */
            iop_lba[dev] = 0xFFFFFFFF;
            return 0;
        }
    }
    if (is_read)
        iop_cmd(0x12);
    else
        iop_cmd(0x13);
    iop_cmd(fd);
    iop_data(0x00);
    iop_data(0x02);	/* 512 bytes */
    if (!is_read)
        iop_data_out(dptr);	/* In common - write the data block */
    if (c = iop_rx())
        goto bad;
    iop_rx();		/* Skip size info */
    iop_rx();
    if (is_read)
        iop_data_in(dptr);    /* Lives in common as may go to user directly */
    iop_lba[dev] = lba + 512;
    return 1;
}	

void iop_probe(void)
{
    int fd;
    register uint_fast8_t i;

    for (i = 0; i < NR_DISK; i++) {
        fd = open_disk(i);
        if (fd >= 0) {
            iop_fd[i] = fd;
            kprintf("FUZIX%u: ", i);
            if (td_register(i, iop_xfer, td_ioctl_none, 1) < 0)
                break;
        }
    }
}

static uint_fast8_t bcd(uint8_t *p)
{
    uint8_t v = p[1] & 0x0F;
    v |= (p[0] & 0x0F) << 4;
    return v;
}

int plt_rtc_read(void)
{
    uint8_t buf[16];
    struct cmos_rtc cmos;
    uint16_t len = sizeof(struct cmos_rtc);
    register uint8_t *p = cmos.data.bytes;
    register uint8_t *bp = buf;
    register unsigned i;

    if (udata.u_count < len)
        len = udata.u_count;

    iop_cmd(0x03);
    iop_read(buf, 16);
    /* 0 YYYY MM DD HH mm ss */
    /* Turn it BCD */
    for (i = 0; i < 8; i++) {
        *p++ = bcd(bp);
        bp += 2;
    }
    cmos.type = CMOS_RTC_BCD;
    if (uput(&cmos, udata.u_base, len) == -1)
        return -1;
    return len;
}

int plt_rtc_write(void)
{
    udata.u_error = EINVAL;
    return -1;
}

static uint8_t js_last[8];
static uint8_t js_mod;
static uint8_t mouse_last[5];
static uint8_t mouse_mod;
/* TODO: probe for */
static uint8_t have_js;
static uint8_t have_mouse;

static uint8_t buf[32];

static struct s_queue kqueue = {
    buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

void plt_input_wait(void)
{
    psleep(&kqueue);
}

static void iop_poll_js(uint8_t *buf)
{
    if (memcmp(buf + 1, js_last, 8)) {
        js_mod = 1;
        memcpy(js_last, buf + 1, 8);
        wakeup(&kqueue);
    }
}

static void iop_poll_mouse(uint8_t *buf)
{
    if (memcmp(buf + 1, mouse_last, 5)) { 
        mouse_mod = 1;
        memcpy(mouse_last, buf + 1, 5);
        wakeup(&kqueue);
    }
}

void poll_input(void)
{
    uint8_t buf[9];

    if (have_js) {
        iop_cmd(0x0E);
        iop_data(0x00);
        iop_read(buf, 9);
        if (buf[0] == 0)
            iop_poll_js(buf);
    }
    if (have_mouse) {
        iop_cmd(0x0C);
        iop_read(buf, 6);
        if (buf[0] == 0)
            iop_poll_mouse(buf);
    }
}

int plt_input_write(uint_fast8_t flag)
{
    udata.u_error = EINVAL;
    return -1;
}

static void scale16(uint8_t *ptr, uint16_t val, uint16_t scale)
{
    val *= scale;
    *ptr++ = val;
    *ptr = val >> 8;
}

int plt_input_read(uint8_t *slot)
{
    uint_fast8_t r, k;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
        *slot++ = KEYPRESS_CODE | r;
        *slot = k;
        return 2;
    }
    if (mouse_mod) {
        *slot++ = MOUSE_ABS;
        /* Scale 0-319 and 0-199 to 16bit range */
        scale16(slot, mouse_last[0] + 256 * mouse_last[1], 205);
        slot += 2;
        scale16(slot, mouse_last[2], 329);
        slot += 2;
        *slot++ = mouse_last[3] & 7;
        /* What to do with wheel ?? */
        *slot++ = mouse_last[4];
        mouse_mod = 0;
        return 7;
    }
    if (js_mod) {
        /* TODO: second stick etc */
        *slot++ = STICK_ANALOG;
        *slot++ = 0;
        *slot++ = js_last[0];
        *slot++ = js_last[1];
        *slot++ = js_last[7];
        js_mod = 0;
        return 5;
    }
    return 0;
}

