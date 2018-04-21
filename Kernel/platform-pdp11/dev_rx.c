/*
 *	Driver for an RX floppy disk controller
 *
 *	77 track, 26 sector per track, 128 byte per sector floppy
 *
 *	Usually lives at 17170/2 (command/status at 0 data at 2)
 *
 *	See "RX8/RX11 Floppy Disk System User's Manual EK-RX01-OP-001"
 */

#include <kernel.h>
#include <printf.h>
#include <dev_rx.h>

#define GO		0x01

#define CMD_FILL	0x00
#define CMD_EMPTY	0x02
#define CMD_WRITE	0x04
#define CMD_READ	0x06
#define CMD_READSTAT	0x08
#define CMD_FORMAT	0x0A	/* Set density (later models only) */
#define CMD_WRITEDEL	0x0C
#define CMD_READERROR	0x0E

#define DRIVE_B		0x10
#define DONE		0x20
#define INTEN		0x40
#define XFERRQ		0x80
#define INTIALIZE	0x4000
#define ERROR		0x8000

static uint16_t *dev = (uint16_t *)0177170;
static uint8_t drive;
static uint8_t track;
static uint8_t sector;

static void issue_read(void)
{
    uint8_t x;
    uint8_t ct = 0;
    /* Might want timeouts here */

    /* Wait for it to stop being busy  */
    while(!(*dev & DONE));	
    *dev = CMD_READ|GO|drive;	/* READ|GO|DRIVE_B etc */
    while(!(*dev & XFERRQ));
    dev[1] = sector;		/* When ready send the sector */
    while(!(*dev & XFERRQ));
    dev[1] = track;		/* And the track */

    /* The controller will no go off and get the sector */
    while(!(*dev & DONE));
    
    /* Now read the buffer */
    *dev = CMD_EMPTY|GO;	/* Ready for xfer */
    while(ct < 128) {
        x = *dev;
        if (x & XFERRQ) {
            *udata.u_dptr++ = dev[1];	/* Store byte */
            ct++:
        }
        /* An early done means an error */
        if (x & DONE)
            break;		/* Error */
    }
    return dev[1];	/* Error bits */
}

static void issue_write(void)
{
    uint8_t x;
    uint8_t ct = 0;
    while(!(*dev & DONE));	/* Make sure we are not busy */

    /* We need to fill the buffer */
    *dev = CMD_FILL|GO;	/* Ready for xfer */
    while(ct < 128) {
        x = *dev;
        if (x & 0x80) {		/* Failure */
            dev[1] = *udata.u_dptr+;	/* Store byte */
            ct++:
        }
        if (x & DONE)
            break;		/* Error */
    }
    /* Error in xfer ? */
    if (dev[0] & (1 << 15)))
        return dev[1];
    /* Now we have copied the buffer into the controller we issue an I/O */
    *dev = CMD_WRITE|GO|drive;	/* READ|GO|DRIVE_B etc */
    while(!(*dev & XFERRQ));
    dev[1] = sector;
    while(!(*dev & XFERRQ));
    dev[1] = track;
    
    /* Wait for the write to complete or error */
    while(!(*dev & DONE));
    return dev[1];	/* Error bits */
}

static int probe_drive(void)
{
    while(!(*dev & DONE));
    *dev = CMD_READSTAT|GO|drive;
    while(!(*dev & DONE));
    return (*dev & 0x80);
}

static int rx_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    int ct = 0;
    int tries;
    uint8_t err = 0;
    uint16_t ptr;

    if(rawflag == 2)
        goto bad2;

    rx_map = rawflag;
    if (rawflag && d_blkoff(BLKSHIFT))
            return -1;

    /* In 128's */
    udata.u_nblock *= 4;
    udata.u_block *= 4;

    drive = (minor & 1) ? DRIVE_B : 0;

    while (ct < udata.u_nblock) {
        track = udata.u_block /26;
        sector = (udata.u_block % 26) + 1;
        for (tries = 0; tries < 4 ; tries++) {
            ptr = udata.u_dptr;
            err = (is_read ? issue_read : issue_write)(dev))
            if (err == 0)
                break;
            udata.u_dptr = ptr;
            if (tries > 1)
                rx_reset(driveptr);
        }
        /* FIXME: should we try the other half and then bale out ? */
        if (tries == 4)
            goto bad;
        ct++;
        udata.u_block++;
    }
    return ct << 7;
bad:
    kprintf("rx%d: error %x\n", minor, err);
bad2:
    udata.u_error = EIO;
    return -1;
}

int rx_open(uint8_t minor, uint16_t flag)
{
    flag;
    drive = (minor & 1) ? DRIVE_B : 0;
    if(minor >= MAX_FD || !probe_drive()) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rx_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return rx_transfer(minor, true, rawflag);
}

int rx_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;rawflag;minor;
    return rx_transfer(minor, false, rawflag);
}
    