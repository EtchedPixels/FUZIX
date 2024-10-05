#include <kernel.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>

#define ide_ctrl	0x04
#define ide_data	0x05
#define porta		0x06
#define portb		0x07

static void ide_set_w(void)
{
    out(portb, 0xCF);
    out(portb, 0x00);
}

static void ide_set_r(void)
{
    out(portb, 0xCF);
    out(portb, 0xFF);
}

uint8_t ide_read(uint_fast8_t r)
{
    uint_fast8_t v;
    /* Active low control lines */
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7));
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_W|PIOIDE_RESET|(r & 7));
    v = in(ide_data);
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7));
    out (ide_ctrl, PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET);
    return v;
}

void ide_write(uint_fast8_t r, uint_fast8_t v)
{
    ide_set_w();
    /* Active low control lines */
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7));
    out(ide_data, v);
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_R|PIOIDE_RESET|(r & 7));
    out(ide_ctrl, PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET|(r & 7));
    out(ide_ctrl, PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET);
    ide_set_r();
}

void ide_pio_setup(void)
{
    /* Set up control port */
    out(ide_ctrl, PIOIDE_CS0|PIOIDE_CS1|PIOIDE_R|PIOIDE_W|PIOIDE_RESET);
    out(porta, 0xCF);
    out(porta, 0x00);
    /* Keep data as read usually */
    ide_set_r();
}

