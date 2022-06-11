/*
 *	CDC HAWK disk controller
 *
 *	This is usually at F1xx but they can also be at other addresses
 *	according to the WIPL probe.
 *
 *	This platform uses 400 byte physical and file system blocks so
 *	some things are a bit strange.
 *
 *	TODO:
 *		Check error and busy handling versus boot/diag
 *		Check DMA config
 *		Figure out DMA and MMU mapping
 *		Bad track handling
 */

#include "kernel.h"
#include "kdata.h"
#include "printf.h"
#include "centurion.h"

static void hawk_unit(uint_fast8_t unit)
{
    *((volatile uint8_t *)0xF140) = unit;
}

static void hawk_sector(uint16_t sector)
{
    *((volatile uint16_t *)0xF141) = sector;
}

static uint16_t hawk_status(void)
{
    return *((volatile uint16_t *)0xF144);
}

static int hawk_command(uint_fast8_t cmd)
{
    uint_fast8_t st;
    *((volatile uint8_t *)0xF148) = cmd;
    /* If this doesn't go straight to setting bit 4 then bail */
    if (!(hawk_status() & 0x10))
        return 1;
    /* Wait for completion bit */
    while((st = hawk_status()) & 0x0100);
    /* Return status bits */
    return st;
}

/* Bad track remap - TODO */
static uint16_t hawk_remap(uint16_t sector)
{
    return sector;
}

static int hawk_seek(uint16_t sector)
{
    uint16_t st;

    sector = hawk_remap(sector);
    hawk_sector(sector);
    if (hawk_command(2))
        return -1;
    do {
        st = hawk_status();
        /* Error ? */
        if (st & 0x0400)
            break;
    } while(!(st & 0x0020));
    /* Seek succeeded */
    if (!(st & 0x0400))
        return 0;
    if (hawk_command(3) == 0) {
        do {
            st = hawk_status();
            /* Error ? */
            if (st & 0x0400)
                break;
        } while(!(st & 0x0020));
    }
    if (st & 0x0400)
        kprintf("hawk: RTZ failed.\n");
    return -1;
}

static int hawk_xfer(uint_fast8_t cmd, uint16_t count)
{
    count *= 400;
    dma_set_mode0();
    dma_enable();
    dma_set_base((uint16_t)udata.u_dptr);
    dma_set_length(0xFFFF - count);
    return hawk_command(cmd) & 0xFF00;
}

static int hawk_transfer(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t cmd)
{
    unsigned tries, nsec;
    switch(rawflag) {
    case 0:	/* Kernel I/O */
        break;
    case 1:	/* Direct to user */
        /* We will need to figure out how the DMA and MMU interact before
           we can support this
        if (d_blkmap())
            return -1;
        */
        panic("ndio");
    case 2:	/* Swap: same issue */
        panic ("ndio");
    }
    udata.u_count = udata.u_nblock;

    /* The hawk is 16 x 400 sectors with the head and cyl bits above, this
       conveniently means we can just ram the value into the controller */
    hawk_unit(minor);
    while (udata.u_count) {
        tries = 0;
        while (tries-- < 5) {
            if (hawk_seek(udata.u_block) == -1)
                continue;
            /* Sectors we can do this run */
            nsec = 16 - (udata.u_block & 15);
            if (nsec > udata.u_count)
                nsec = udata.u_count;
            if (hawk_xfer(cmd, nsec) == 0) {
                udata.u_count -= nsec;
                udata.u_block += nsec;
                break;
            }
        }
        if (tries == 0) {
            kputs("hawk: I/O error.\n");
            break;
        }
    }
    return (udata.u_nblock - udata.u_count) * 400;
}

int hawk_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return hawk_transfer(minor, rawflag, 0);
}

int hawk_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return hawk_transfer(minor, rawflag, 1);
}

/* Partitioning to follow */
int hawk_open(uint_fast8_t minor, uint16_t flags)
{
    if (minor > 1) {
        udata.u_error = ENXIO;
        return -1;
    }
    return 0;
}

int hawk_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
    unsigned long size = 16 * 810;
    if (request != BLKGETSIZE)
        return -1;
    return uput(&size, data, sizeof(long));
}
