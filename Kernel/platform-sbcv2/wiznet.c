#include <kernel.h>

/*
 *	Drive a WizNet 5100 using indirect mode at ports 0x28-0x2B
 */

#ifdef CONFIG_NET_WIZNET

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5100.h>

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
uint8_t w5100_readb(uint16_t off)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	return idm_dr;
}

uint16_t w5100_readw(uint16_t off)
{
	uint16_t n;
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	n = ((uint16_t)idm_dr) << 8;
	n |= idm_dr;
	return n;
}

void w5100_bread(uint16_t off, uint8_t *p, uint16_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		*p++ = idm_dr;
}

void w5100_breadu(uint16_t off, uint8_t *p, uint16_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		uputc(idm_dr, p++);
}

void w5100_writeb(uint16_t off, uint8_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	idm_dr = n;
}

void w5100_writew(uint16_t off, uint16_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	idm_dr = n >> 8;
	idm_dr = n;
}

void w5100_bwrite(uint16_t off, uint8_t *p, uint16_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		idm_dr = *p++;
}

void w5100_bwriteu(uint16_t off, uint8_t *p, uint16_t n)
{
	idm_ar0 = off >> 8;
	idm_ar1 = off;
	while(n--)
		idm_dr = ugetc(p++);
}

void w5100_setup(void)
{
	mr = MR_AUTOINC|MR_INDIRECT;
}

#endif
