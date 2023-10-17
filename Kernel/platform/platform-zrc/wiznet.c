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

#define	mr	0x28
#define idm_ar0	0x29
#define idm_ar1	0x2A
#define idm_dr	0x2B

/* We assume indirect, autoinc is always set */
uint8_t w5x00_readcb(uint16_t off)
{
	irqflags_t irq = di();
	uint8_t r;
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	r = in(idm_dr);
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
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	n = ((uint16_t)in(idm_dr)) << 8;
	n |= in(idm_dr);
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
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	while(n--)
		*p++ = in(idm_dr);
	irqrestore(irq);
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	out(idm_ar0, off >> 8);
	out (idm_ar1, off);
	while(n--)
		uputc(in(idm_dr), p++);
	irqrestore(irq);
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
	irqflags_t irq = di();
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	out(idm_dr, n);
	irqrestore(irq);
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
	return w5x00_writecb((sock << 8) + off, n);
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	out(idm_dr, n >> 8);
	out(idm_dr, n);
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
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	while(n--)
		out(idm_dr, *p++);
	irqrestore(irq);
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	irqflags_t irq = di();
	uint8_t *p = pv;
	off += bank;
	out(idm_ar0, off >> 8);
	out(idm_ar1, off);
	while(n--)
		out(idm_dr, ugetc(p++));
	irqrestore(irq);
}

void w5x00_setup(void)
{
	out(mr, MR_AUTOINC|MR_INDIRECT);
}

#endif
