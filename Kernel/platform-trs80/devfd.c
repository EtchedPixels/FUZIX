/* 
 * TRS80 disk driver (TODO)
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#define FD_TIMEOUT	100		/* FIXME */

/* Floppy controller */
__sfr __at 0xF0	fd_command;
__sfr __at 0xF0	fd_status;
#define FD_BUSY		1
#define FD_DRQ		2

#define CMD_RESET	0x0B
#define CMD_SEEK	0x1B
#define CMD_READ	0x8C
#define CMD_WRITE	0xAC
__sfr __at 0xF1 fd_track;
__sfr __at 0xF2 fd_sector;
__sfr __at 0xF3 fd_data;
/* Drive select */
__sfr __at 0xF4 fd_select;

/* floppies. 26 128 byte sectors, not a nice way to use all of them in 512's */
static int sectrack[16] = {
    18
};

static uint8_t track[4];	/* only one controller register for all 4 drives */
static uint8_t curdrive = 0xFF;
static uint8_t fd_timer;

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag);

/* Replace with proper asm delay */
static void nap(void)
{
    int i;
    for(i=0;i<16;i++);
}

/* To write */
static int fd_wait_idle(void)
{
    return 0xFF;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(true, minor, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return fd_transfer(false, minor, rawflag);
}

static uint8_t fd_seek(uint8_t track)
{
    uint8_t status;
    fd_track = track;
    nap();
    fd_command = CMD_SEEK;
    nap();
    status = fd_wait_idle();
    /* FIXME: bits of status */
    return 0;
}

static uint8_t fd_writedata(uint8_t *dptr)
{
    uint8_t status;
    irqflags_t irq;
    uint8_t r;
    uint16_t a;
    
    fd_command = CMD_WRITE;
    nap();
    do {
        r = fd_status;
        if (r & FD_DRQ) {
            irq = di();
            for (a = 0; a < 256; a++) {
                fd_data = *dptr++;
            }
            irqrestore(irq);
            status = fd_wait_idle();
            /* bits of status */
            return status & 0x5C;
        }
    }
    while (r & FD_BUSY);
    /* Went clear without asking for data */
    return -1;
}

static uint8_t fd_readdata(uint8_t *dptr)
{
    uint8_t status;
    irqflags_t irq;
    uint8_t r;
    unsigned int a;
    
    fd_command = CMD_READ;
    nap();
    do {
        r = fd_status;
        if (r & FD_DRQ) {
            irq = di();
            for (a = 0; a < 256; a++) {
                *dptr++= fd_data;;
            }
            irqrestore(irq);
            status = fd_wait_idle();
            /* bits of status */
            return status & 0x1C;
        }
    }
    while (r & FD_BUSY);
    /* Went clear without asking for data */
    return -1;
}

static uint8_t fd_reset(void)
{
    uint8_t status;
    
    fd_command = CMD_RESET;
    nap();
    status = fd_wait_idle();
    return 0;
}

static uint8_t fd_geom(int minor, blkno_t block)
{
    /* Turn block int track/sector 
       and write to the controller.
       Forced to do real / and % */
    uint8_t trackw = block / sectrack[minor];
    uint8_t sectorw = block % sectrack[minor];
    uint8_t status = 0;

    if (trackw != track[curdrive]) {
        status = fd_seek(trackw);
        track[curdrive] = trackw;
    }
    fd_sector = sectorw;
    fd_timer = FD_TIMEOUT;
    nap();
    return status & 0x10;
}

/* Deselect drive, motor off, may be called from an IRQ */
static void fd_deselect(void)
{
    if (curdrive != 0xFF) {
        track[curdrive] = fd_track;
        curdrive = 0xFF;
    }
    fd_select = 0;
}

static void sdcc_bug(void)
{
}

static void fd_drivesel(uint8_t minor)
{
    irqflags_t irq = di();
    uint8_t sdcc_tmp;

    if (minor != curdrive) {
        if (curdrive != 0xFF)
            fd_deselect();
        track[curdrive] = fd_track;
        /* nap needed anywhere ? */
        fd_track = track[minor];
        curdrive = minor;
        /* FIXME: check and check for spin up times */

        /* Do this in two steps to stop SDCC 3.4 crashing and
           we need the dummy call to stop it optimising it back into
           the broken version */
        sdcc_tmp = 1 << minor;
        sdcc_bug();
        fd_select = sdcc_tmp;
    }
    fd_timer = FD_TIMEOUT;
    irqrestore(irq);
}

static int fd_transfer(bool is_read, uint8_t minor, uint8_t rawflag)
{
    blkno_t block;
    int block_xfer;     /* r/w return value (number of 512 byte blocks transferred) */
    uint8_t *dptr;
    int dlen;
    int ct = 0;
    int st;
    int tries;

    if(rawflag) {
        dlen = udata.u_count;
        dptr = udata.u_base;
        block = udata.u_offset >> 9;
        block_xfer = dlen >> 8;		/* 256 byte blocks */
    } else { /* rawflag == 0 */
        dlen = 512;
        dptr = udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        block_xfer = 2;
    }

    fd_drivesel(minor);
    while (ct < block_xfer) {
        for (tries = 0; tries < 3; tries++) {
            fd_geom(minor, block);
            if (tries > 0)
                fd_reset();
            if (is_read)
                st = fd_readdata(dptr);
            else
                st = fd_writedata(dptr);
            if (st == 0)
                break;
        }
        if (tries == 3)
            kprintf("fd%d: disk error %02X\n", st);
        block++;
        ct++;
        dptr += 256;
    }
    return ct/2;
}

int fd_open(uint8_t minor)
{
    if(minor >= 3 || !sectrack[minor]) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}
