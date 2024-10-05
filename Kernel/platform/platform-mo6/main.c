#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <devinput.h>
#include <sd.h>

uint8_t membanks;
uint8_t in_bios;
uint16_t swap_dev;
uint8_t inputdev;
uint8_t inputwait;
uint8_t vblank_wait;

static uint8_t intct;

void plt_idle(void)
{
    irqflags_t irq = di();
    if (!in_bios)
        poll_keyboard();
        if (++intct == 5) {
	    timer_interrupt();
	    intct = 0;
	    if (inputwait)
	        wakeup(&inputwait);
        }
    if (vblank_wait)
        wakeup(&vblank_wait);
    irqrestore(irq);
}

uint8_t plt_param(char *p)
{
    return 0;
}

void do_beep(void)
{
}

void plt_discard(void)
{
}

void plt_interrupt(void)
{
}

uint8_t vtattr_cap;

/* SD glue */


uint_fast8_t sd_type = 0;

void sd_spi_tx_byte(uint8_t b)
{
//    kprintf(">%2x", b);
    switch(sd_type) {
    case SDIF_SDDRIVE:
        sddrive_tx_byte(b);
        break;
    case SDIF_SDMOTO:
        sdmoto_tx_byte(b);
        break;
    case SDIF_SDMO:
        sdmo_tx_byte(b);
        break;
    }
}

uint8_t sd_spi_rx_byte(void)
{
    uint8_t r;
    switch(sd_type) {
    case SDIF_SDDRIVE:
        r = sddrive_rx_byte();
        break;
    case SDIF_SDMOTO:
        r = sdmoto_rx_byte();
        break;
    case SDIF_SDMO:
        r = sdmo_rx_byte();
        break;
    }
//    kprintf("<%2x", r);
    return r;
}

void sd_spi_tx_sector(uint8_t *ptr)
{
    switch(sd_type) {
    case SDIF_SDDRIVE:
        sddrive_tx_sector(ptr);
        break;
    case SDIF_SDMOTO:
        sdmoto_tx_sector(ptr);
        break;
    case SDIF_SDMO:
        sdmo_tx_sector(ptr);
        break;
    }
}

void sd_spi_rx_sector(uint8_t *ptr)
{
    uint16_t n;
    switch(sd_type) {
    case SDIF_SDDRIVE:
        sddrive_rx_sector(ptr);
        break;
    case SDIF_SDMOTO:
        sdmoto_rx_sector(ptr);
        break;
    case SDIF_SDMO:
        sdmo_rx_sector(ptr);
        break;
    }
#if 0
    for (n = 0; n < 512; n++) {
        kprintf("%2x ", *ptr++);
        if ((n & 7) == 7)
            kprintf("\n");
    }
#endif
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

void sd_spi_raise_cs(void)
{
}

void sd_spi_lower_cs(void)
{
}
