#include <kernel.h>
#include <enc28j60.h>
#include <z80softspi.h>
#include <printf.h>

/*
 *	Platform level code for our EN28J60. Currently attached via an I/O
 *	board for convenience.
 */

#define ENC_PORT		0x03

#define IN_DATA			0x80

#define OUT_CS			0x01
#define OUT_RESET		0x02
#define OUT_CLOCK		0x04
#define OUT_DATA		0x80

__sfr __at ENC_PORT	enc_gpio;

uint8_t enc_gpio_state;

/* In case we share the port later */
static void enc_set_gpio(uint8_t m, uint8_t v)
{
    enc_gpio_state &= ~m;
    enc_gpio_state |= v;
    enc_gpio = enc_gpio_state;
}

void enc_nap_1ms(void)
{
    /* Wait 1ms : FIXME tune this */
    volatile unsigned int n = 0;
    while(--n);
}

void enc_reset(void)
{
    enc_set_gpio(OUT_RESET, 0);
    enc_nap_1ms();
    enc_set_gpio(OUT_RESET, OUT_RESET);
}

void enc_select(void)
{
    enc_set_gpio(OUT_CS, 0);
}

void enc_deselect(void)
{
    /* Delay ? */
    enc_set_gpio(OUT_CS, OUT_CS);
}

void enc_read_block(uint8_t *buf, unsigned int len)
{
    enc_select();
    spi_transmit_byte(READBUF_CMD);
    /* Check if RX sending FF is ok or we need to send 00 */
    while(len--)
        *buf++ = spi_receive_byte();
    enc_deselect();
}

/* Probably belongs mostly in caller helper for speed */
void enc_read_block_user(uint8_t *buf, unsigned int len)
{
    enc_select();
    spi_transmit_byte(READBUF_CMD);
    /* Check if RX sending FF is ok or we need to send 00 */
    while(len--)
        uputc(spi_receive_byte(), buf++);
    enc_deselect();
}

void devenc_init(void)
{
    spi_port = ENC_PORT;
    spi_piostate = 0x00;
    spi_data = OUT_DATA;
    spi_clock = OUT_CLOCK;
    
    enc_init();
}
