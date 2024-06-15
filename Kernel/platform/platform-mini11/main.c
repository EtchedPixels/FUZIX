#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <netdev.h>
#include <net_w5x00.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint8_t need_resched;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
    irqflags_t flags = di();
//    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}


/* Our E clock is 2MHz, the prescaler divides it by 16 so the
   timr counts at 125Khz giving us 2500 pulses per 50th */

#define CLK_PER_TICK	2500

uint16_t timer_step = CLK_PER_TICK;

/* This is incremented at 50Hz by the asm code */
uint8_t timer_ticks;

/* Each timer event we get called after the asm code has set up the timer
   again. ticks will usually be incremented by one each time, but if we
   miss a tick the overflow logic may make ticks larger */
void plt_event(void)
{
	tty_poll();
	/* We can't poll the Wiznet if the SD card is mid transaction */
	if (tinysd_busy == 0)
		w5x00_poll();

	/* Turn the 50Hz counting into a 50Hz tty poll and a 10Hz
	   timer processing */
	while(timer_ticks >= 5) {
		timer_interrupt();
		timer_ticks -= 5;
	}
}


int strcmp(const char *s1, const char *s2)
{
  char c1, c2;

  while((c1 = *s1++) == (c2 = *s2++) && c1);
  return c1 - c2;
}

/*
 *	WizNET 5500 Glue
 *
 *	Need to look at the IRQ handling but may be stuck with this
 *	because whilst we can avoid an spi poll during an spi transaction
 *	we've got a whole raft of fun with PA7 being in the same register
 *	as the page bits.
 */

#define _SLOT(x)	((x) << 2)
#define SOCK2BANK_C(x)	((_SLOT(x) | 1) << 3)
#define SOCK2BANK_W(x)	((_SLOT(x) | 2) << 3)
#define SOCK2BANK_R(x)	((_SLOT(x) | 3) << 3)
#define W_WRITE	0x04

extern void spi_select_port(unsigned port);
extern void spi_select_none(void);

/* We can optimize this lot later when it works nicely */
static void spi_transaction(uint8_t ctrl, uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	sd_spi_tx_byte(off >> 8);
	sd_spi_tx_byte(off);
	sd_spi_tx_byte(ctrl);
	while(outlen--)
		sd_spi_tx_byte(*out++);
	while(inlen--)
		*in++ = sd_spi_rx_byte();
	spi_select_none();
	irqrestore(irq);
}

static void spi_transaction_u(uint8_t ctrl, uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	sd_spi_tx_byte(off >> 8);
	sd_spi_tx_byte(off);
	sd_spi_tx_byte(ctrl);
	while(outlen--)
		sd_spi_tx_byte(_ugetc(out++));
	while(inlen--)
		_uputc(sd_spi_rx_byte(), in++);
	spi_select_none();
	irqrestore(irq);
}

uint8_t w5x00_readcb(uint16_t off)
{
	uint8_t r;
	spi_transaction(1, off, NULL, 0, &r, 1);
	return r;
}

uint8_t w5x00_readsb(uint8_t s, uint16_t off)
{
	uint8_t r;
	spi_transaction(SOCK2BANK_C(s)| 1, off, NULL, 0, &r, 1);
	return r;
}

uint16_t w5x00_readcw(uint16_t off)
{
	uint16_t r;
	spi_transaction(2, off, NULL, 0, (uint8_t *)&r, 2);
	return ntohs(r);
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
	uint16_t r;
	spi_transaction(SOCK2BANK_C(s)| 2, off, NULL, 0, (uint8_t *)&r, 2);
	return ntohs(r);
}

void w5x00_bread(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank, off, NULL, 0, pv, n);
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction_u(bank, off, NULL, 0, pv, n);
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
	spi_transaction(1 | W_WRITE, off, &n, 1, NULL, 0);
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
	spi_transaction(SOCK2BANK_C(sock) | 1 | W_WRITE, off, &n, 1, NULL, 0);
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(2 | W_WRITE, off, (uint8_t *)&n, 2, NULL, 0);
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(SOCK2BANK_C(sock) | 2 | W_WRITE, off, (uint8_t *)&n, 2, NULL, 0);
}

void w5x00_bwrite(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank|W_WRITE, off, pv, n, NULL, 0);
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction_u(bank|W_WRITE, off, pv, n, NULL, 0);
}

void w5x00_setup(void)
{
	w5x00_writecb(0, 0x80);
}
