#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#define  ppi_port_a *((volatile uint8_t *)0xC00001)	/* LSB */
#define  ppi_port_b *((volatile uint8_t *)0xC00003)	/* MSB */
#define  ppi_port_c *((volatile uint8_t *)0xC00005)	/* IDE Control */
#define  ppi_control *((volatile uint8_t *)0xC00007)	/* 82C55 Control */

void ppide_init(void)
{
    ppi_control = PPIDE_PPI_BUS_READ;
    ppi_port_c = ide_reg_status;
}

uint8_t devide_readb(uint8_t regaddr)
{
    uint8_t r;

    /* note: ppi_control should contain PPIDE_PPI_BUS_READ already */
    ppi_port_c = regaddr;
    ppi_port_c = regaddr | PPIDE_RD_LINE; /* begin /RD pulse */
    r = ppi_port_a;
    ppi_port_c = regaddr;	 /* end /RD pulse */
    return r;
}

void devide_writeb(uint8_t regaddr, uint8_t value)
{
    ppi_control = PPIDE_PPI_BUS_WRITE;
    ppi_port_c = regaddr;
    ppi_port_a = value;
    ppi_port_b = 0;
    ppi_port_c = regaddr | PPIDE_WR_LINE;
    /* FIXME: check timing */
    ppi_port_c = regaddr;
    ppi_control = PPIDE_PPI_BUS_READ;
}
