#include <kernel.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>

__sfr __at 0xB4 ide_ctrl;
__sfr __at 0xB5 ide_data;
__sfr __at 0xB6 porta;
__sfr __at 0xB7 portb;

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
    ide_ctrl = PIOIDE_R|PIOIDE_W|(r & 7);
    ide_ctrl = PIOIDE_W|(r & 7);
    v = ide_data;
    ide_ctrl = PIOIDE_R|PIOIDE_W|(r & 7);
    ide_ctrl = PIOIDE_CS0|PIOIDE_R|PIOIDE_W;
    return v;
}

void ide_write(uint8_t r, uint_fast8_t v)
{
    ide_set_w();
    /* Active low control lines */
    ide_ctrl = PIOIDE_R|PIOIDE_W|(r & 7);
    ide_data = v;
    ide_ctrl = PIOIDE_R|(r & 7);
    ide_ctrl = PIOIDE_R|PIOIDE_W|(r & 7);
    ide_ctrl = PIOIDE_CS0|PIOIDE_R|PIOIDE_W;
    ide_set_r();
}

void ide_pio_setup(void)
{
    /* Set up control port */
    ide_ctrl = PIOIDE_CS0|PIOIDE_R|PIOIDE_W;
    porta = 0xCF;
    /* Bits 0 and 1 are internal IRQ lines */
    porta = 0x03;
    /* Set up int vector */
    porta = 0x00;
    /* Now set up interrupts */
    porta = 0xA7;
    /* For now just the serial */
    porta = 0x02;
    /* Keep data as read usually */
    ide_set_r();
}
