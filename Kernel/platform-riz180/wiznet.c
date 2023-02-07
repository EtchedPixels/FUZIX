#include <kernel.h>
#include <riz180.h>
#include <devsd.h>
/*
 *	Drive a WizNet 5200 or 5500 over SPI
 */

#ifdef CONFIG_NET_WIZNET

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5x00.h>

#ifdef CONFIG_NET_W5200
#define W_WRITE	0x80

/* We can optimize this lot later when it works nicely */
static void spi_transaction(uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	spi_send(off >> 8);
	spi_send(off);
	if (outlen) {
		spi_send((outlen >> 8) | W_WRITE);
		spi_send(outlen);
		while(outlen--)
			spi_send(*out++);
	} else {
		spi_send(inlen >> 8);
		spi_send(inlen);
		
		while(inlen--)
			*in++ = spi_recv();
	}
	spi_select_none();
	irqrestore(irq);
}

static void spi_transaction_u(uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	spi_send(off >> 8);
	spi_send(off);
	if (outlen) {
		spi_send((outlen >> 8) | W_WRITE);
		spi_send(outlen);
		while(outlen--)
			spi_send(_ugetc(out++));
	} else {
		spi_send(inlen >> 8);
		spi_send(inlen);
		
		while(inlen--)
			_uputc(spi_recv(), in++);
	}
	spi_select_none();
	irqrestore(irq);
}

uint8_t w5x00_readcb(uint16_t off)
{
	uint8_t r;
	spi_transaction(off, NULL, 0, &r, 1);
	return r;
}

uint8_t w5x00_readsb(uint8_t s, uint16_t off)
{
	uint8_t r;
	spi_transaction((s << 8) + off, NULL, 0, &r, 1);
	return r;
}

uint16_t w5x00_readcw(uint16_t off)
{
	uint16_t r;
	spi_transaction(off, NULL, 0, &r, 2);
	return ntohs(r);
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
	uint16_t r;
	spi_transaction((s << 8) + off, NULL, 0, &r, 2);
	return ntohs(r);
}

void w5x00_bread(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank + off, NULL, 0, pv, n);
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction_u(bank + off, NULL, 0, pv, n);
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
	spi_transaction(off, &n, 1, NULL, 0); 
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
	spi_transaction((sock << 8) + off, &n, 1, NULL, 0);
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(off, &n, 2, NULL, 0); 
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction((sock << 8) + off, &n, 2, NULL, 0); 
}

void w5x00_bwrite(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank + off, pv, n, NULL, 0);
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction_u(bank + off, pv, n, NULL, 0);
}

void w5x00_setup(void)
{
}
#else /* 5500 */
#define _SLOT(x)	((x) << 2)
#define SOCK2BANK_C(x)	((_SLOT(x) | 1) << 3)
#define SOCK2BANK_W(x)	((_SLOT(x) | 2) << 3)
#define SOCK2BANK_R(x)	((_SLOT(x) | 3) << 3)
#define W_WRITE	0x04

/* We can optimize this lot later when it works nicely */
static void spi_transaction(uint8_t ctrl, uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	spi_send(off >> 8);
	spi_send(off);
	spi_send(ctrl);
	while(outlen--)
		spi_send(*out++);
	while(inlen--)
		*in++ = spi_recv();
	spi_select_none();
	irqrestore(irq);
}

static void spi_transaction_u(uint8_t ctrl, uint16_t off,
	uint8_t *out, uint16_t outlen, uint8_t *in, uint16_t inlen)
{
	irqflags_t irq = di();
	spi_select_port(2);
	spi_send(off >> 8);
	spi_send(off);
	spi_send(ctrl);
	while(outlen--)
		spi_send(_ugetc(out++));
	while(inlen--)
		_uputc(spi_recv(), in++);
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
	spi_transaction(2, off, NULL, 0, &r, 2);
	return ntohs(r);
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
	uint16_t r;
	spi_transaction(SOCK2BANK_C(s)| 2, off, NULL, 0, &r, 2);
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
	spi_transaction(2 | W_WRITE, off, &n, 2, NULL, 0); 
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(SOCK2BANK_C(sock) | 2 | W_WRITE, off, &n, 2, NULL, 0); 
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

#endif
#endif
