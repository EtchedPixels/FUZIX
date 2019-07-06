#include <kernel.h>
#include <kdata.h>
#include <devide.h>
#include <printf.h>

__sfr __at 0xF5 ide_addr;
__sfr __at 0xF6 ide_high;
__sfr __at 0xF7 ide_low;

uint8_t atom_type;

uint_fast8_t devide_readb(uint_fast8_t reg)
{
    volatile uint8_t dummy;
    ide_addr = reg;
    dummy = ide_high;
    return ide_low;
}

void devide_writeb(uint_fast8_t reg, uint_fast8_t value)
{
    ide_addr = reg;
    ide_high = 0;
    ide_low = value;
}

uint8_t atom_probe(void)
{
    devide_writeb(ide_reg_devhead, 0xE0);
    ide_addr = ide_reg_lba_0;
    ide_high = 0xAA;
    ide_low = 0x55;
    /* This triggers the read back. On an AtomLite it reads the lba register
       and returns it. On the Atom it reads the lba register and latches it
       and returns the high 8bits - ie 0 */
    if (ide_high == 0x55) {
        kputs("AtomLite IDE detected.\n");
        atom_type = ATOM_LITE;
        return 1;
    }
    /* Did we latch 0x55. If so we have a real Atom IDE */
    if (ide_low == 0x55) {
        kputs("Atom IDE detected.\n");
        atom_type = ATOM_16BIT;
        return 1;
    }
    return 0;
}
