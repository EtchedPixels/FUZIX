/* 
 * ROMdisc hack for testing
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devhd.h>
#include <io.h>

extern uint8_t hd_kmap;

extern void hd_read_data(uint16_t addr);
extern void hd_write_data(uint16_t addr);

static int hd_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t dptr, nb;
    irqflags_t irq;
    uint8_t err;

    /* FIXME: add swap */
    if(rawflag == 1 && d_blkoff(9))
        return -1;

    /* For swap it'll be the swap bank passed */
    hd_kmap = rawflag ? udata.u_page : KERNEL_BANK;

    dptr = (uint16_t)udata.u_dptr;
    nb = udata.u_nblock;

    while (udata.u_nblock--) {
        poke(0x3000|minor);
        poke(0x3100|(udata.u_block >> 8));
        poke(0x3200|(udata.u_block & 0xFF));
        poke(0x3301);
        if ((err = peek(0x35)) != 0) {
            kprintf("hd%d: disk error %x\n", minor, err);
            udata.u_error = EIO;
            return -1;
        }
        /* We shouldn't need the di any more */        
        irq = di();
        if (is_read)
            hd_read_data(dptr);
        else
            hd_write_data(dptr);
        irqrestore(irq);
        udata.u_block++;
        dptr += 512;
    }
    return nb << BLKSHIFT;
}

int hd_open(uint8_t minor, uint16_t flag)
{
    uint8_t err;

    used(flag);
    err = peek(0x35);
    poke(0x3300|minor);
    if(err) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(minor, true, rawflag);
}

int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(flag);
    return hd_transfer(minor, false, rawflag);
}

