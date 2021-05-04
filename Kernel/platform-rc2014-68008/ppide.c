#include <kernel.h>
#include <blkdev.h>
#include <devide.h>

static volatile uint8_t *io = (uint8_t *)0x10000;

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
    io[0x23] = PPIDE_PPI_BUS_READ;
    io[0x22] = PPIDE_REG_STATUS;
}

uint_fast8_t ppide_readb(uint_fast8_t p)
{
    uint8_t r;
    io[0x22] = p | 0x08;
    io[0x22] = p | 0x08 | PPIDE_RD_LINE;	/* 0x80 ? */
    r = io[0x20];
    io[0x22] = p | 0x08;
    return r;
}

void ppide_writeb(uint8_t p, uint_fast8_t v)
{
    io[0x23] = PPIDE_PPI_BUS_WRITE;
    io[0x22] = p | 0x08;
    io[0x20] = v;
    io[0x21] = 0;
    io[0x22] = p | PPIDE_WR_LINE | 0x08;
    io[0x22] = p | 0x08;
    io[0x23] = PPIDE_PPI_BUS_READ;
}

/* Flat memory model so this is not too difficult */

void ppide_read_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    while(ct--) {
        io[0x22] = PPIDE_REG_DATA|PPIDE_RD_LINE|0x08;
        *p++ = io[0x20];
        *p++ = io[0x21];
        io[0x22] = PPIDE_REG_DATA|0x08;
    }
}

void ppide_write_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    io[0x22] = PPIDE_REG_DATA | 0x08;
    io[0x23] = PPIDE_PPI_BUS_WRITE;
    while(ct--) {
        io[0x20] = *p++;
        io[0x21] = *p++;
        io[0x22] = PPIDE_REG_DATA|PPIDE_WR_LINE;
        io[0x22] = PPIDE_REG_DATA | 0x08 ;
    }
    io[0x23] = PPIDE_PPI_BUS_READ;
}    

#endif
