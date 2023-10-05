/*
 *	Disk driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devices.h>
#include <devfd.h>
#include <sio.h>

#define FD_RESET	0x0D
#define FD_READ		0x77
#define FD_WRITE	0x78
#define FD_WRITEHST	0x79	/* Flush buffer */
#define FD_COPY		0x7A	/* Offline copy */
/* 0x7B appears to have been redacted from docs ? */
#define FD_FORMAT	0x7C	/* Format disk */

#define FD_OK		0x00
#define FD_ERR_READ	0xFA
#define FD_ERR_WRITE	0xFB
#define FD_ERR_DRIVESEL	0xFC
#define FD_ERR_WPROT	0xFD

/* Worst case size */
static uint8_t buf[0x89];

static void fd_reset(uint8_t minor)
{
    buf[0] = 0x00;
    buf[1] = 0x30 + minor >> 1;
    buf[2] = 0x23;
    buf[3] = 0x0D;
    buf[4] = 0x00;
    buf[5] = 0x00;
    sio_write(buf, 6);
    sio_read(buf, 6);
    if (buf[5])
        kprintf("fd%d: reset error %d\n", minor, buf[5]);
}

int fd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    blkno_t block;
    uint16_t dptr;
    uint16_t ct;
    uint8_t tries;
    uint16_t nblock;
    staticfast uint8_t page;
    staticfast int old_sio;
    staticfast int err;

    ct = 0;

    if (rawflag == 0) {
	dptr = (uint16_t)udata.u_buf->bf_data;
	block = udata.u_buf->bf_blk << 2 ;
	nblock = 4;
	page = 0;		/* Kernel */
    } else if (rawflag == 1) {
	if (((uint16_t)udata.u_offset|udata.u_count) & BLKMASK)
		goto bad2;
	dptr = (uint16_t)udata.u_base;
	nblock = udata.u_count >> 7;
	block = udata.u_offset >> 7;	/* 128 bytes/sector */
	page = udata.u_page;		/* User */
    } else if (rawflag == 2) {
	nblock = swapcnt >> 7;	/* in 128 byte chunks */
	dptr = (uint16_t)swapbase;
	page = swappage;
	block = swapblk << 2;
    } else
	goto bad;


    old_sio = select_sio();

    /* Loop over each 128 byte chunk */
    while (ct < nblock) {
        buf[0] = 0x00;
        buf[1] = 0x31 + minor >> 1; 	/* Which FDD ? */
        buf[2] = 0x23;
        buf[6] = (block >> 6);		/* 64 logical sectors/track */
        buf[7] = (block & 63) + 1;	/* Sectors are 1 based */

        for (tries = 0; tries < 4; tries ++) {
            if (is_read) {
                /* Write command to the drive, wait for reply */
                buf[3] = FD_READ;
                buf[4] = 0x02;
                sio_write(buf, 8);
                err = sio_read(buf, 0x86);
                if (!err) {
                    err = buf[0x86];
                    if (!err)
                        /* Buffer goes to user space for this process or
                           to kernel. We always page in ourself */
                        if (rawflag)
                            _uput(buf + 0x05, (uint8_t *)dptr, 128);
                        else
                            memcpy((uint8_t *)dptr, buf + 0x05, 128);
                }
            } else {
                /* Write the buffer and data, then wait for an ack */
                buf[3] = FD_WRITE;
                buf[4] = 0x83;
                buf[8] = (((uint8_t)block) & 3) == 3 ? 0x00 : 0x02;
                /* We need a special I/O helper for swapping out as we may
                   be swapping memory from an I/O mapped ram device to
                   the floppy disc */
                if (rawflag == 0)
                    memcpy(buf + 0x09, (uint8_t *)dptr, 128);
                else {
                    romd_off = dptr;
                    romd_addr = (uint16_t)buf + 0x09;
                    romd_size = 128;
                    romd_mode = page;
                    read_from_bank();
                }
                sio_write(buf, 0x89);
                err = sio_read(buf, 0x06);
                if (!err)
                    err = buf[0x05];
            }
            /* If it worked no more retries */
            if (err == 0)
                break;
            /* Errors not worth a retry */
            if (err >= FD_ERR_DRIVESEL)
                goto bad2;
            /* Failed ? */
            if (tries == 3)
                goto bad2;
            if (tries > 1)
                fd_reset(minor);
        }
        /* Move on 128 bytes */
        block++;
        dptr += 128;
        ct++;
    }
    deselect_sio(old_sio);
    return ct >> 2;
bad2:
    deselect_sio(old_sio);
bad:
    kprintf("fd%d: I/O error %d.\n", minor, err);
    udata.u_error = EIO;
    return -1;
}

int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return fd_transfer(minor, true, rawflag);
}

int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return fd_transfer(minor, false, rawflag);
}

int fd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor >= 3) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

static void mdelay(uint16_t d)
{
    used(d);
    /* FIXME */
}

__sfr __at 0x19 rom_ctrl;

static void rom_on(void)
{
    mod_ioctrlr(0x08,0x00);	/* Cartridge out of reset */
    mdelay(10);
    rom_ctrl = 1;		/* Power on */
    mdelay(60);
}

static void rom_off(void)
{
    rom_ctrl = 0;
    mod_ioctrlr(0x00,0x08);
}

/* hd is actually the ROM image files */
int rom_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);

    if(rawflag == 1) {
        romd_size = udata.u_count;
        romd_addr = (uint16_t)udata.u_base;
        romd_off = udata.u_offset >> 8;
        /* Should check this higher up ? */
        if (((uint16_t)udata.u_offset | romd_size) & BLKMASK)
            goto bad;
    } else if (rawflag == 2) {
        goto bad;
    } else { /* rawflag == 0 */
        romd_addr = (uint16_t)udata.u_buf->bf_data;
        romd_off = udata.u_buf->bf_blk << 1;
        romd_size = 512;
    }
    romd_mode = rawflag;
    if (minor == 0)
        rom_sidecar_read();
    else {
        rom_on();
        rom_cartridge_read();
        rom_off();
    }
    return romd_size;
bad:
    udata.u_error = EIO;
    return -1;
}


int rom_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(minor);
    used(rawflag);
    used(flag);

    udata.u_error = EIO;
    return -1;
}

int rom_open(uint8_t minor, uint16_t flag)
{
    flag;
    switch (minor) {
        case 0:
            if (sidecar)
                return 0;
            break;
        case 1:
            if (carttype == 1)	/* ROM */
                return 0;
            break;
    }
    udata.u_error = ENODEV;
    return -1;
}
