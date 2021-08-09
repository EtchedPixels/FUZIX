#include <kernel.h>

/*
 *	Drive a WizNet 5100 using indirect mode at ports 0x28-0x2B
 */

#ifdef CONFIG_NET_WIZNET

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5x00.h>

#define MR		0x0000
#define		MR_RESET	0x80
#define		MR_PB		0x10
#define		MR_PPPOE	0x08
#define		MR_AUTOINC	0x02
#define		MR_INDIRECT	0x01

/* Core helpers: platform supplied */

__sfr __at 0x28 mr;
__sfr __at 0x29 idm_ar0;
__sfr __at 0x2A idm_ar1;
__sfr __at 0x2B idm_dr;

/* We assume indirect, autoinc is always set */
uint8_t w5x00_readcb(uint16_t off)
{
	irqflags_t irq = di();
	uint8_t r;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	r = idm_dr;
	irqrestore(irq);
	return r;
}

uint8_t w5x00_readsb(uint8_t s, uint16_t off)
{
	off += ((uint16_t)s) << 8;
	return w5x00_readcb(off);
}

uint16_t w5x00_readcw(uint16_t off)
{
	irqflags_t irq = di();
	uint16_t n;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	n = ((uint16_t)idm_dr) << 8;
	n |= idm_dr;
	irqrestore(irq);
	return n;
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
	off += ((uint16_t)s) << 8;
	return w5x00_readcw(off);
}

void w5x00_bread(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		*p++ = idm_dr;
	irqrestore(irq);
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		uputc(idm_dr, p++);
	irqrestore(irq);
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
	irqflags_t irq = di();
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	idm_dr = n;
	irqrestore(irq);
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
	return w5x00_writecb((sock << 8) + off, n);
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	idm_dr = n >> 8;
	idm_dr = n;
	irqrestore(irq);
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
	return w5x00_writecw((sock << 8) + off, n);
}

void w5x00_bwrite(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		idm_dr = *p++;
	irqrestore(irq);
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		idm_dr = ugetc(p++);
	irqrestore(irq);
}

void w5x00_setup(void)
{
	mr = MR_AUTOINC|MR_INDIRECT;
}

#endif
