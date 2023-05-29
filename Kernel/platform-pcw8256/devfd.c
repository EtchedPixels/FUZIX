/*
 *	Amstrad PCW8256 Floppy Driver
 *
 *	Probe routines. These set everything up for the generic fdc765 driver
 *	to do the real work for us. We should try and move most of this into
 *	discard.
 *
 *	TODO:
 *	Need to sort out seek rate programming (here and in 765 driver core)
 *	765 drive asm needs hooks for fd_send()/fd_intwait()
 *	Ensure fdc interrupt is off
 *	ioctls for double step and hook into core 765 ?
 *	FDC ioctl hooks in general
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devfd.h>
#include <devfdc765.h>
#include <platform_fdc765.h>
#include <pcw8256.h>

__sfr __at 0xf8 asic_ctrl;
__sfr __at 0x00 fdc_c;
__sfr __at 0x01 fdc_d;

uint8_t new_fdc;

static uint8_t track[2] = { 255, 255 };
static uint8_t sides[2];
static uint8_t type[2];
#define TYPE_NONE	0
#define TYPE_THREE	1
#define TYPE_THREE_FIVE	2
#define TYPE_UNKNOWN	3
#define TYPE_DS		4

uint8_t fdc765_present, fdc765_ds;
uint8_t diskmotor;

static uint8_t fd_send(uint8_t cmd, uint8_t minor)
{
    uint8_t *stat;

    fd765_drive = minor;

    stat = fd765_send_cmd(&cmd);
    return *stat;
}

/*
 *	The Amstrad 3" drives are 30ms head settle,  12ms step
 *	480ms head unload, 4ms head load, 1.75ms write current off
 *
 *	Amstrad seems to use 1 second motor stabilizing time and 5 second
 *	motor off.
 *
 *	Amstrad actually appears to use 28ms settle.
 *
 *	The NEC765 is clocked at 4MHz so the step timer is in 2ms increments
 *	the head load in 4ms intervals, and the HUT is in 32ms intervals.
 */
static const uint8_t setup_cmd[] = {
    0x03,		/* SPECIFY */
    0xaf,		/* SRT and HUT - 12ms seek */
    0x03,		/* ND, head load time */
};

static void fd_setup(void)
{
    fd765_send_cmd3(setup_cmd);
}

/* FIXME: Want an fdc discard */

static void fd_probe_plus(int d)
{
    int du = 2 * d;
    uint8_t st;
    fd765_motor_on();
    st = fd_send(0x07, du);
    st = fd765_intwait();
    st = fd_send(0x04, du);
    /* Check track 0 */
    /* Did it report trk0 */
    if (!(st & 0x10)) {
        /* Not set: r/o means 3 r/w 3.5, for slave means no drive */
        if (st & 0x40)
            type[d] = TYPE_THREE;
        else if (d == 0)
            type[d] = TYPE_THREE_FIVE;
        else
            type[d] = TYPE_NONE;
    } else {
        asic_ctrl = 0x10;
        /* motor wait needed */
        fd765_motor_off();
        /* Wait for not ready status */
        do {
            st = fd_send(0x04, 0);
        } while (st & 0x20);
        st = fd_send(0x04, du);
        if (st & 0x10)
            type[d] = TYPE_THREE;
        else
            type[d] = TYPE_THREE_FIVE;
    }
    if (st & 8)
        type[d] |= TYPE_DS;
    /* Motors back off */
    fd765_motor_off();
}

static const char *fdnames[] = {
    NULL,
    "180K",
    "3.5\"",
    "??",
    NULL,
    "720K",
    "3.5\"",
    "??",
    NULL
};

void fd_probe(void)
{
    uint8_t i;
    uint8_t st;
    uint8_t m;

    /* Motor off */
    fd765_motor_off();

    /* Wait for not ready status */
    do {
        st = fd_send(0x04, 0);
    } while (st & 0x20);
    /* Get status of 'drive 2' */
    st = fd_send(0x04, 2);
    if (st & 0x20) {
        new_fdc = 1;
        /* Reports ready - new style drive */
        fd_probe_plus(0);
        fd_probe_plus(1);
    }
    else {
        /* FIXME: can we detect 3.5" on older machines. Need to play about */
        st = fd_send(0x04, 0);
        type[0] = TYPE_THREE | ((st & 8) ? TYPE_DS : 0);
        fd765_motor_on();
        st = fd_send(0x07, 1);
        st = fd765_intwait();
        st = fd_send(0x04, 1);
        if (st & 0x10) {
            type[1] = TYPE_THREE | ((st & 8) ? TYPE_DS : 0);
            sides[1] = (st & 8) ? 2 : 1;
        }
        /* FIXME: should do the 3 or 3.5 check generically - 3.5 won't
           trk0 if not ready */
    }
    fd765_motor_off();

    machine_ident();

    m = 1;
    for (i = 0; i < 2; i++) {
        const char *p = fdnames[type[i]];
        if (p == NULL)
            type[i] = TYPE_NONE;
        else {
            if (type[i] & TYPE_DS)
                fdc765_ds |= m;
            fdc765_present |= m;
            kprintf("fd%d: %s\n", i, p);
        }
        m <<= 1;
    }
    fd_setup();
}
