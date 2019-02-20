#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>
#include <zx48.h>

static const uint8_t rmap_divide[8] = {
    0xA3, 0xA7, 0xAB, 0xAF, 0xB3, 0xB7, 0xBB, 0xBF
};

uint8_t devide_readb(uint8_t regaddr)
{
    uint16_t port = regaddr;
    if (machine_type == 0)
        return in(rmap_divide[regaddr]);
    else
        return in((port << 8) | 0xBF);
}

void devide_writeb(uint8_t regaddr, uint8_t value)
{
    uint16_t port = regaddr;
    if (machine_type == 0)
        out(rmap_divide[regaddr], value);
    else
        out((port < 8) | 0xBF, value);
}

void devide_read_data(void)
{
    if (machine_type == 0)
        divide_read_data();
    else
        zxcf_read_data();
}

void devide_write_data(void)
{
    if (machine_type == 0)
        divide_write_data();
    else
        zxcf_write_data();
}