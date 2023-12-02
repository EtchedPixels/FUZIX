#include <kernel.h>

/*
 *	Driver support for Wiznet 5100 and 5300 carrier modules. The
 *	5500 on SPI is currently only enabled on the rcbus-z180 platform
 */

/*
 *	Drive a WizNet 5100 using indirect mode at ports 0x28-0x2B
 */

#ifdef CONFIG_NET

#include <kdata.h>
#include <printf.h>
#include <netdev.h>

#ifdef CONFIG_NET_W5100
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

#else

/* Wiznet 5300 has its own interface */
#include <net_w5300.h>

#define mr0	0x28
#define mr1	0x29
#define idm_arh	0x2A
#define idm_arl	0x2B
#define idm_drh	0x2C
#define idm_drl	0x2D

/*
 * Not well documented: the drh/drl pair must always be used high then low,
 * as if they are read the other way around the FIFO gets confused
 */

/* Read a W5300 register in big endian order */
uint16_t w5300_read(uint16_t off)
{
	irqflags_t irq = di();
	uint16_t r;
	out(idm_arh, off >> 8);
	out(idm_arl, off);
	r = in(idm_drh) << 8;
	r |= in(idm_drl);
	irqrestore(irq);
	return r;
}

/* Read a W5300 register without byte swapping */
uint16_t w5300_readn(uint16_t off)
{
	irqflags_t irq = di();
	uint16_t r;
	out(idm_arh, off >> 8);
	out(idm_arl, off);
	r = in(idm_drh);
	r |= in(idm_drl) << 8;
	irqrestore(irq);
	return r;
}

/* Write a W5300 register in big endian order */
void w5300_write(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	out(idm_arh, off >> 8);
	out(idm_arl, off);
	out(idm_drh, n >> 8);
	out(idm_drl, n);
	irqrestore(irq);
}

/* Write a W5300 register without byte swapping */
void w5300_writen(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	out(idm_arh, off >> 8);
	out(idm_arl, off);
	out(idm_drh, n);
	out(idm_drl, n >> 8);
	irqrestore(irq);
}

/*
 *	The core W5300 code expects the FIFO to be native endian and the
 *	registers to be kept big endian. We also need to set extended hold
 *	because 7ns data hold is a bit too quick for the poor old Z80.
 */
void w5300_setup(void)
{
	uint16_t i;
	out(mr1, 0x80);	/* Reset */
	for (i = 0; i < 1000; i++);
	out(mr0, in(mr0) | 0x05);	/* FIFO in little endian, extended hold */
	out(mr1, 0x01);	/* Indirect mode */
}

#endif
#endif
