/*
 *	Wayne Warthen's PropIO 2.
 *
 *	ECB Bus version (although it'll work whatever the bus so long as it's
 *	I/O mapped at 0xA8). In other words *not* the parportprop
 *
 *	This will not work if you are booting straight into Fuzix without a
 *	ROM probing it first. The PropIO takes a few seconds (ROMWBW waits 4)
 *	to respond after power up. As we always boot through ROMWBW on any
 *	platform currently using this code we don't bother waiting again.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tinydisk.h>
#include <tty.h>
#include <propio2.h>

__sfr __at 0xA8	tstatus;
__sfr __at 0xA9 tdata;
__sfr __at 0xAA dcmd;
__sfr __at 0xAA dstatus;
__sfr __at 0xAB dio;

#define TBUSY		0x80
#define TERR		0x40
#define TKBD		0x20
#define TDSP		0x10

#define	CMD_NOP		0x00
#define CMD_STAT	0x01
#define CMD_TYPE	0x02
#define CMD_CAP		0x03
#define CMD_CSD		0x04
#define CMD_RESET	0x10
#define CMD_INIT	0x20
#define CMD_READ	0x30
#define CMD_PREP	0x40
#define CMD_WRITE	0x50
#define CMD_VER		0xF0

#define DBUSY		0x80
#define DERR		0x40
#define DOVER		0x20
#define DTO		0x10

void prop_tty_poll(uint8_t minor)
{
    /* Keyboard data pending but not busy. Busy is ok - we'll pick it up
       next poll */
    while((tstatus & (TKBD|TBUSY)) == TKBD) {
        uint8_t r = tdata;
        tty_inproc(minor, r);
    }
}

void prop_tty_write(uint8_t c)
{
    tdata = c;
}

ttyready_t prop_tty_writeready(void)
{
    /* Same basic idea - output ready and not busy */
    return ((tstatus & (TDSP|TBUSY)) == TDSP) ? TTY_READY_NOW : TTY_READY_SOON;
}

/*
 *	It may be an SD card interface but it doesn't look like one so we
 *	don't want to use the SD card layer here
 */

static uint32_t prop_sd_capacity;
static uint32_t prop_sd_error;
static uint8_t prop_sd_status;

/* An error leaves four bytes waiting in the buffer for us to fetch. We don't
   care what they mean right now but just print them. We should care for
   read-only FIXME */
static void prop_error(int st)
{
    /* Recover the error bytes and bitch */
    uint8_t *p = (uint8_t *)&prop_sd_error;
    *p++ = dio;
    *p++ = dio;
    *p++ = dio;
    *p = dio;
    prop_sd_status = st;
    kprintf("propIO sd error: %d %lx\n", st, prop_sd_error);
}

/* Wait for the Propeller to go idle, or time out
   FIXME: delay values - including for slow I/O stuff */
static int prop_idle(void)
{
    uint16_t n;
    /* Delay wants tuning */
    for (n = 0; n < 10000; n++) {
        uint8_t s = dstatus;
        if (!(s & DBUSY))
            return s;
    }
    prop_sd_error = 0xFFFFFFFFUL;
    return -1;
}

/* Send a command to the Propeller */
static int8_t prop_send_cmd(uint8_t cmd)
{
    int st;
    /* Ensure we've finished whatever is going on */
    if (prop_idle() < 0)
        return -1;
    /* Issue our command */
    dcmd = cmd;
    /* Wait for it to go idle again. This needs review. I think it's safe
       for us to skip the second wait in some cases - notably a write command
       where we don't care because the next command will wait for it and
       pipeline better FIXME */
    if ((st = prop_idle()) < 0)
        return -1;
    if (st)
        prop_error(st);	/* Fetch 4 bytes of error code */
    return st;
}

/* Send a command and get data back. Various commands return a block of
   1 to 16 bytes of returned information */
static int8_t prop_send_get(uint8_t cmd, uint8_t *buf, uint8_t l)
{
    uint8_t st;
    st = prop_send_cmd(cmd);
    if (st != 0)
        return st;
    while(l--)
        *buf++ = dio;
    return 0;
}

/* Query and report the card error status */
static int prop_stat(void)
{
    uint8_t stat;
    if (prop_send_get(CMD_STAT, &stat, 1) < 0)
        return -1;
    return stat;
}

/* Reset the controller if present. We ought to check the version here ?? */
static int8_t prop_sd_reset(void)
{
    return prop_send_cmd(CMD_RESET);
}

/* Wrapper for when we open an sd interface. Send an init command. get the
   capacity and report home. -1 means it broke, 0 means we think there isn't
   any media, 1 means all is good */
int8_t prop_sd_open(void)
{
    uint8_t type;
    int8_t r;
    /* It lives... */
    r = prop_send_cmd(CMD_INIT);
    if (r < 0)
        return 0;	/* No media probably */
    /* Type and capacity */
    if (prop_send_get(CMD_TYPE, &type, 1) < 0)
        return -1;
    if (prop_send_get(CMD_CAP, &prop_sd_capacity, 4) < 0)
        return -1;
    return 1;
}

/* Right now this is a no-op but when we queue write we want to ensure th
   write has finished here so we just wait for busy to clear */
int prop_sd_flush_cache(void)
{
    return prop_idle();
}

/*
 * Low level read and write logic. This is fairly simple. We set up the LBA
 * and we then either issue READ and get 512 bytes of data, or we issue
 * PREP, fill the buffer, and WRITE to send it.
 *
 * (or of course anywhere along the line it errors on us)
 *
 * The platform needs to provide platform specific common hooks to inir
 * or otir 512 bytes from the right bank
 */
int prop_sd_xfer(uint8_t dev, bool is_read, uint32_t lba, uint8_t * dptr)
{
    uint8_t *p = &lba;	/* Sadly SDCC sucks at this otherwise */
    uint8_t cmd;

    /* Need to track and handle no media and media changes */

    /* Reset data pointer */
    if (prop_send_cmd(CMD_NOP) < 0)
        return -1;

    dio = *p++;
    dio = *p++;
    dio = *p++;
    dio = *p;

    /* LBA loaded */
    if (is_read)
        cmd = CMD_READ;
    else
        cmd = CMD_PREP;

    if (prop_send_cmd(cmd) < 0)
        return 0;

    /* Now do the transfer via the platform specific helper */
    if (is_read)
        plt_prop_sd_read(dptr);
    else {
        plt_prop_sd_write(dptr);
        if (prop_send_cmd(CMD_WRITE) < 0)
            return 0;
    }
    return 1;
}

/* FIXME: this could move to discard space */

uint8_t prop_sd_probe(void)
{
    if (prop_sd_reset())
        return 0;
    /* Ok we have something. For now do the open here. We need to tweak
       this a bit and make blk support removable media */
    if (prop_sd_open() < 0)
        return 0;

    kputs("PropIO SD: ");
    td_register(0, prop_sd_xfer, td_ioctl_none, 1);
    return 1;
}
