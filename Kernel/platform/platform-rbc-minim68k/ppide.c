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
#include "mfpic.h"
#include "platform_ide.h"

#ifdef PPIDE_BASE
#undef PPIDE_BASE
#define PPIDE_BASE PPORT
#endif

#define PPORT	mf_ppi

#define regA	(PPORT+0)
#define regB	(PPORT+1)
#define regC	(PPORT+2)
#define regCTRL	(PPORT+3)




#define PPIDE_REG_DATA		(PPIDE_CS0_LINE)
#define PPIDE_REG_STATUS	(PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)

void ppide_init(void)
{
    out(regCTRL, PPIDE_PPI_BUS_READ);
    out(regC, PPIDE_REG_STATUS);
}

uint_fast8_t ppide_readb(uint_fast8_t p)
{
    uint8_t r;
    out(regC, p | PPIDE_CS0_LINE);
    out(regCTRL, 1 | (PPIDE_RD_BIT << 1)); /* begin /RD pulse */	
    r = in(regA);
    out(regCTRL, 0 | (PPIDE_RD_BIT << 1)); /* end /RD pulse  */
    return r;
}

void ppide_writeb(uint8_t p, uint_fast8_t v)
{
    out(regCTRL, PPIDE_PPI_BUS_WRITE);
    out(regC, p | PPIDE_CS0_LINE);
    out(regA, v);
/*    out(regB, 0);  */
    out(regCTRL, 1 | (PPIDE_WR_BIT << 1)); /* begin /WR pulse */
    out(regCTRL, 0 | (PPIDE_WR_BIT << 1)); /* end /WR pulse */
    out(regCTRL, PPIDE_PPI_BUS_READ);
}

/* Flat memory model so this is not too difficult */

void ppide_read_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    while(ct--) {
        out(regC, PPIDE_REG_DATA|PPIDE_RD_LINE|PPIDE_CS0_LINE);
        *p++ = in(regA);
        *p++ = in(regB);
        out(regC, PPIDE_REG_DATA|PPIDE_CS0_LINE);
    }
    out(regC, 0);   /* drop CS0 */
}

void ppide_write_data(void)
{
    unsigned int ct = 256;
    uint8_t *p = blk_op.addr;
    out(regC, PPIDE_REG_DATA | PPIDE_CS0_LINE);
    out(regCTRL, PPIDE_PPI_BUS_WRITE);
    while(ct--) {
        out(regA, *p++);
        out(regB, *p++);
        out(regC, PPIDE_REG_DATA | PPIDE_WR_LINE | PPIDE_CS0_LINE);
        out(regC, PPIDE_REG_DATA | PPIDE_CS0_LINE);
    }
    out(regC, 0);	/* drop CS0 line */
    out(regCTRL, PPIDE_PPI_BUS_READ);	/* maintain general state of INPUT */
}    

#endif
