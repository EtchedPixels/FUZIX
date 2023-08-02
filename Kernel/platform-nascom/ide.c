#include <kernel.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>

__sfr __at 0x04 ide_ctrl;
__sfr __at 0x05 ide_data;
__sfr __at 0x06 porta;
__sfr __at 0x07 portb;

static void ide_set_w(void)
{
    portb = 0xCF;
    portb = 0x00;
}

static void ide_set_r(void)
{
    portb = 0xCF;
    portb = 0xFF;
}

uint8_t ide_read(uint8_t r)
{
    uint8_t v;
    /* Active low control lines */
    ide_ctrl = PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7);
    ide_ctrl = PIOIDE_CS1|PIOIDE_W|PIOIDE_RESET|(r & 7);
    v = ide_data;
    ide_ctrl = PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7);
    ide_ctrl = PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET;
    return v;
}

void ide_write(uint8_t r, uint_fast8_t v)
{
    ide_set_w();
    /* Active low control lines */
    ide_ctrl = PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7);
    ide_data = v;
    ide_ctrl = PIOIDE_CS1|PIOIDE_R|PIOIDE_RESET|(r & 7);
    ide_ctrl = PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7);
    ide_ctrl = PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET;
    ide_set_r();
}

void ide_pio_setup(void)
{
    /* Set up control port */
    ide_ctrl = PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET;
    porta = 0xCF;
    porta = 0x00;
    /* Keep data as read usually */
    ide_set_r();
}

