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
#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x28B8 mr;
__sfr __banked __at 0x29B8 idm_ar0;
__sfr __banked __at 0x2AB8 idm_ar1;
__sfr __banked __at 0x2BB8 idm_dr;
#else
__sfr __at 0x28 mr;
__sfr __at 0x29 idm_ar0;
__sfr __at 0x2A idm_ar1;
__sfr __at 0x2B idm_dr;
#endif

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
	uint8_t i;
	mr = 0x80;
	mr = MR_AUTOINC|MR_INDIRECT;
}

#else

/* Wiznet 5300 has its own interface */
#include <net_w5300.h>

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x28B8	mr0;
__sfr __banked __at 0x29B8	mr1;
__sfr __banked __at 0x2AB8	idm_arh;
__sfr __banked __at 0x2BB8 	idm_arl;
__sfr __banked __at 0x2CB8	idm_drh;
__sfr __banked __at 0x2DB8	idm_drl;
#else
__sfr __at 0x28	mr0;
__sfr __at 0x29	mr1;
__sfr __at 0x2A idm_arh;
__sfr __at 0x2B idm_arl;
__sfr __at 0x2C idm_drh;
__sfr __at 0x2D idm_drl;
#endif

/*
 * Not well documented: the drh/drl pair must always be used high then low,
 * as if they are read the other way around the FIFO gets confused
 */

/* Read a W5300 register in big endian order */
uint16_t w5300_read(uint16_t off)
{
	irqflags_t irq = di();
	uint16_t r;
	idm_arh = off >> 8;
	idm_arl = off;
	r = idm_drh << 8;
	r |= idm_drl;
	irqrestore(irq);
	return r;
}

/* Read a W5300 register without byte swapping */
uint16_t w5300_readn(uint16_t off)
{
	irqflags_t irq = di();
	uint16_t r;
	idm_arh = off >> 8;
	idm_arl = off;
	r = idm_drh;
	r |= idm_drl << 8;
	irqrestore(irq);
	return r;
}

/* Write a W5300 register in big endian order */
void w5300_write(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	idm_arh = off >> 8;
	idm_arl = off;
	idm_drh = n >> 8;
	idm_drl = n;
	irqrestore(irq);
}

/* Write a W5300 register without byte swapping */
void w5300_writen(uint16_t off, uint16_t n)
{
	irqflags_t irq = di();
	idm_arh = off >> 8;
	idm_arl = off;
	idm_drh = n;
	idm_drl = n >> 8;
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
	mr1 = 0x80;	/* Reset */
	for (i = 0; i < 1000; i++);
	mr0 |= 0x05;	/* FIFO in little endian, extended hold */
	mr1 = 0x01;	/* Indiredct mode */
}

#endif
#endif
