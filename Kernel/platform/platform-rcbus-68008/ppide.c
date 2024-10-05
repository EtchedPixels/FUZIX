#include <kernel.h>
#include <blkdev.h>
#include <devide.h>

#ifndef CONFIG_MULTI_IDE
#define ppide_readb		devide_readb
#define ppide_writeb		devide_writeb
#define ppide_read_data		devide_read_data
#define ppide_write_data	devide_write_data
#endif

#ifdef CONFIG_PPIDE

#define PPIDE_REG_DATA		0
#define PPIDE_REG_STATUS	7

void ppide_init(void)
{
    out(0x23, PPIDE_PPI_BUS_READ);
    out(0x22, PPIDE_REG_STATUS);
}

uint_fast8_t ppide_readb(uint_fast8_t p)
{
    uint8_t r;
    out(0x22, p | 0x08);
    out(0x22, p | 0x08 | PPIDE_RD_LINE);
    r = in(0x20);
    out(0x22, p | 0x08);
    return r;
}

void ppide_writeb(uint8_t p, uint_fast8_t v)
{
    out(0x23, PPIDE_PPI_BUS_WRITE);
    out(0x22, p | 0x08);
    out(0x20, v);
    out(0x21, 0);
    out(0x22, p | PPIDE_WR_LINE | 0x08);
    out(0x22, p | 0x08);
    out(0x23, PPIDE_PPI_BUS_READ);
}

/* Flat memory model so this is not too difficult */

void ppide_read_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    while(ct--) {
        out(0x22, PPIDE_REG_DATA|PPIDE_RD_LINE|0x08);
        *p++ = in(0x20);
        *p++ = in(0x21);
        out(0x22, PPIDE_REG_DATA|0x08);
    }
}

void ppide_write_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    out(0x22, PPIDE_REG_DATA | 0x08);
    out(0x23, PPIDE_PPI_BUS_WRITE);
    while(ct--) {
        out(0x20, *p++);
        out(0x21, *p++);
        out(0x22, PPIDE_REG_DATA|PPIDE_WR_LINE);
        out(0x22, PPIDE_REG_DATA | 0x08);
    }
    out(0x23, PPIDE_PPI_BUS_READ);
}    

#endif
