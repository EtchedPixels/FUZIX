/* 
 *	RAMdisc driver
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devrd.h>

static uint8_t rd_present = 2;		/* Unknown */

/* Hooks to asm interface */
extern uint8_t rd_page;
extern uint8_t *rd_dptr;
extern uint16_t rd_block;
extern void ramread5(void);
extern void ramwrite5(void);
extern uint8_t ramdet5(void);

static int rd_transfer(bool is_read, uint_fast8_t rawflag)
{
    unsigned ct = 0, num;

    if(rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
    }

    rd_page = rawflag;
    rd_dptr = udata.u_dptr;
    rd_block = udata.u_block;

    num = udata.u_nblock;

    /* We must be careful here. Our stack on a swap is private but we are
       going to overwrite udata in some cases */
    while (ct < num) {
        if (rd_block > 511)
            break;
        if (is_read)
            ramread5();
        else
            ramwrite5();
        /* rd_dptr is advanced by the asm helpers */
        rd_block++;
        ct++;
    }
    return ct << BLKSHIFT;
}

int rd_open(uint_fast8_t minor, uint16_t flag)
{
    flag;
    if(minor != 0 || !rd_present) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return rd_transfer(true, rawflag);
}

int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return rd_transfer(false, rawflag);
}

void rd_probe(void)
{
    rd_present = ramdet5();
    if (rd_present)
        kputs("RAMdisc found at 0x58.\n");
}
