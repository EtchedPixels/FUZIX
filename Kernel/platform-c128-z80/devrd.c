/* 
 *	RAMdisc driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

static uint16_t rd_size[2];

/* Hooks to asm interface */
extern uint8_t rd_page;
extern uint8_t *rd_dptr;
extern uint16_t rd_block;

extern void geo_read(void);
extern void geo_write(void);
extern unsigned geo_probe(void);
extern void reu_read(void);
extern void reu_write(void);
extern unsigned reu_probe(void);

static int rd_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    unsigned ct = 0, num;
    void (*op)(void);

    if(rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
    }

    rd_page = rawflag;
    rd_dptr = udata.u_dptr;
    rd_block = udata.u_block;

    num = udata.u_nblock;

    if (minor) {
        if (is_read)
            op = reu_read;
        else
            op = reu_write;
    } else {
        if (is_read)
            op = geo_read;
        else
            op = geo_write;
    }
    /* We must be careful here. Our stack on a swap is private but we are
       going to overwrite udata in some cases */
    while (ct < num) {
        if (rd_block >= rd_size[minor]) {
            udata.u_error = EIO;
            break;
        }
        op();
        /* rd_dptr is advanced by the asm helpers */
        rd_block++;
        ct++;
    }
    return ct << BLKSHIFT;
}

int rd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    /* Cheaper to add than shift for the 0/1 to 1/2 case only */
    if(minor > 1 || rd_size[minor] == 0) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return rd_transfer(minor, true, rawflag);
}

int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;
    return rd_transfer(minor, false, rawflag);
}

void rd_probe(void)
{
    rd_size[0] = reu_probe();
    if (rd_size[0])
        kprintf("Probed %d KB REU.\n", rd_size[0] >> 1);
    rd_size[1] = geo_probe();
    if (rd_size[1])
        kprintf("Probed %d KB GeoRAM.\n", rd_size[1] >> 1);
}
