#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <blkdev.h>
#include "dev/devsd.h"
#include "esp8266_peri.h"
#include "globals.h"
#include "config.h"

#define RESETPIN 2			/* also change GPF2 and GPC2 below */
#define CSPIN 	4			/* also change GPF4 and GPC4 below */
#define CSPIN2	5			/* also change GPF5 and GPC5 below */

typedef union {
	uint32_t regValue;
	struct {
		unsigned regL:6;
		unsigned regH:6;
		unsigned n:6;
		unsigned pre:13;
		unsigned regEQU:1;
	};
} spiclock_t;

void sd_rawinit(void)
{
	GPF2 = GPFFS(GPFFS_GPIO(RESETPIN));	/* Reset for W5500 */
	GPF4 = GPFFS(GPFFS_GPIO(CSPIN));	/* CS */
	GPF5 = GPFFS(GPFFS_GPIO(CSPIN2));	/* CS for second device */
	GPF14 = GPFFS(GPFFS_BUS(14));		/* CLK */
	GPF13 = GPFFS(GPFFS_BUS(13));		/* MTCK, MOSI */
	GPF12 = GPFFS(GPFFS_BUS(12));		/* MTDI, MISO */
	GPC2 = 0;		/* GPIO_DATA, normal driver, interrupt disabled, wakeup disabled */
	GPC4 = 0;		/* GPIO_DATA, normal driver, interrupt disabled, wakeup disabled */
	GPC5 = 0;		/* GPIO_DATA, normal driver, interrupt disabled, wakeup disabled */
	GPC16 = 0;		/* GPIO_DATA, normal driver, interrupt disabled, wakeup disabled */
	GPES = (1 << CSPIN) | (1 << CSPIN2) | (1 << RESETPIN);

	SPI1C = 0;
	SPI1U = 0;
	SPI1P = 0;
	SPI1U1 = 0;
	SPI1C1 = 0;
	SPI1S = 0;
}

static uint32_t calculate_freq(spiclock_t * reg)
{
	return ((PERIPHERAL_CLOCK * 1000000)
		/ ((reg->pre + 1) * (reg->n + 1)));
}

static void set_clock(uint32_t clockDiv)
{
	if (clockDiv == 0x80000000)
		GPMUX |= (1 << 9);
	else
		GPMUX &= ~(1 << 9);
	SPI1CLK = clockDiv;
}

/* This code is borrowed heavily from the Arduino code at
 * https://github.com/esp8266/Arduino/blob/master/libraries/SPI/SPI.cpp
 * I don't understand how it works --- the clock computation appears
 * to be invalid. But it does. */

static void setFrequency(uint32_t freq)
{
	const uint32_t clock = PERIPHERAL_CLOCK * 1000000;
	if (freq >= clock) {
		/* magic number to set spi sysclock bit (see below.) */
		set_clock(0x80000000);
		return;
	}

	const spiclock_t minFreqReg = { 0x7FFFF020 };
	uint32_t minFreq = calculate_freq((spiclock_t *) & minFreqReg);
	if (freq < minFreq) {
		// use minimum possible clock regardless
		set_clock(minFreqReg.regValue);
		return;
	}

	spiclock_t best_reg = { 0 };
	int32_t best_freq = 0;

	// aka 0x3F, aka 63, max for n:6
	const uint8_t nMax = (1 << 6) - 1;

	// aka 0x1fff, aka 8191, max for pre:13
	const int32_t preMax = (1 << 13) - 1;

	// find the best match for the next 63 iterations
	for (uint8_t n = 1; n <= nMax; n++) {
		spiclock_t reg = { 0 };
		int32_t actual_freq;
		int32_t pre;

		reg.n = n;

		for (int8_t i = -1; i <= 1; i++) {
			pre = (((clock / (reg.n + 1)) / freq) - 1) + i;
			if (pre > preMax)
				reg.pre = preMax;
			else if (pre <= 0)
				reg.pre = 0;
			else
				reg.pre = pre;

			reg.regL = ((reg.n + 1) / 2);
			// reg.regH = (reg.n - reg.regL);

			// test calculation
			actual_freq = calculate_freq(&reg);
			//kprintf("-----[%p][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d = %d\n", reg.regValue, freq, reg.regEQU, reg.pre, reg.n, reg.regH, reg.regL, actual_freq);

			if (actual_freq == (int32_t) (freq)) {
				// accurate match use it!
				memcpy(&best_reg, &reg, sizeof(best_reg));
				break;
			} else if (actual_freq < (int32_t) (freq)) {
				// never go over the requested frequency
				int32_t cal = abs((int32_t) (freq) - actual_freq);
				int32_t best = abs((int32_t) (freq) - best_freq);
				if (cal < best) {
					best_freq = actual_freq;
					memcpy(&best_reg, &reg, sizeof(best_reg));
				}
			}
		}
		if (actual_freq == (int32_t) (freq))
			break;
	}

	//kprintf("[%p][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d\t - Real Frequency: %d\n", best_reg.regValue, freq, best_reg.regEQU, best_reg.pre, best_reg.n, best_reg.regH, best_reg.regL, calculate_freq(&best_reg));

	set_clock(best_reg.regValue);
}

/*
 *	We may have two devices that run at different clocks.
 */
static uint8_t clk_fast[2] = { 0xFF, 0xFF };
static uint8_t last_clk;

static void sd_spi_reclock(unsigned int port)
{
	if (last_clk == clk_fast[port])
		return;
	last_clk = clk_fast[port];
	setFrequency(last_clk ? 4000000 : 250000);
}

static void spi_set_clock(unsigned int port, bool go_fast)
{
	clk_fast[port] = go_fast;
	sd_spi_reclock(port);
}

void sd_spi_clock(bool go_fast)
{
	spi_set_clock(sd_drive, go_fast);
}

static void spi_deselect_port(unsigned int port)
{
	if (port == 0)
		GPOS = 1 << CSPIN;
	else
		GPOS = 1 << CSPIN2;
	sd_spi_reclock(port);
}

static void spi_select_port(unsigned int port)
{
	if (port == 0)
		GPOC = 1 << CSPIN;
	else
		GPOC = 1 << CSPIN2;
	sd_spi_reclock(port);
}

void sd_spi_raise_cs(void)
{
	spi_deselect_port(sd_drive);
}

void sd_spi_lower_cs(void)
{
	spi_select_port(sd_drive);
}

static uint_fast8_t send_recv(uint_fast8_t data)
{
	while (SPI1CMD & SPIBUSY);

	SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
	SPI1U = SPIUMOSI | SPIUDUPLEX;
	SPI1W0 = data;
	SPI1CMD = SPIBUSY;

	while (SPI1CMD & SPIBUSY);

	return SPI1W0 & 0xff;
}

void sd_spi_transmit_byte(uint_fast8_t b)
{
	send_recv(b);
}

uint_fast8_t sd_spi_receive_byte(void)
{
	return send_recv(0xff);
}

/*
 *	Copy an aligned 32bit block to another aligned target. We use this because we need to
 *	write dwords to and from the SPI FIFO.
 */

static void memcpy4_aligned(uint32_t *p1, const uint32_t *p2, uint32_t dwords)
{
	while(dwords--)
		*p1++ = *p2++;
}

bool sd_spi_receive_sector(void)
{
	uint8_t *dptr = (uint8_t *) blk_op.addr;
	if ((uint32_t) dptr & 3)
		panic("unaligned read");
	for (int chunk = 0; chunk < 8; chunk++) {
		while (SPI1CMD & SPIBUSY);

		SPI1U1 = (511 << SPILMISO);
		SPI1U = SPIUMISO;
		SPI1CMD = SPIBUSY;

		while (SPI1CMD & SPIBUSY);

		memcpy4_aligned((void *)dptr, (const void *) &SPI1W0, 16);
		dptr += 64;
	}

	return 0;
}

bool sd_spi_transmit_sector(void)
{
	uint8_t *sptr = (uint8_t *) blk_op.addr;
	if ((uint32_t) sptr & 3)
		panic("unaligned write");
	for (int chunk = 0; chunk < 8; chunk++) {
		while (SPI1CMD & SPIBUSY);

		SPI1U1 = (511 << SPILMOSI);
		SPI1U = SPIUMOSI;
		memcpy4_aligned((void *) &SPI1W0, (const void *)sptr, 16);
		SPI1CMD = SPIBUSY;

		while (SPI1CMD & SPIBUSY);

		sptr += 64;
	}

	return 0;
}

#ifdef CONFIG_NET_WIZNET

/*
 *	Wiznet 5500 glue to the SPI layer
 */

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
	spi_select_port(1);
	send_recv(off >> 8);
	send_recv(off);
	send_recv(ctrl);
	while(outlen--) {
		send_recv(*out++);
	}
	while(inlen--) {
		*in = send_recv(0xFF);
		in++;
	}
	spi_deselect_port(1);
	irqrestore(irq);
}

uint8_t w5x00_readcb(uint16_t off)
{
	uint8_t r;
	spi_transaction(0, off, NULL, 0, &r, 1);
	return r;
}

uint8_t w5x00_readsb(uint8_t s, uint16_t off)
{
	uint8_t r;
	spi_transaction(SOCK2BANK_C(s), off, NULL, 0, &r, 1);
	return r;
}

uint16_t w5x00_readcw(uint16_t off)
{
	uint16_t r;
	spi_transaction(0, off, NULL, 0, (uint8_t *)&r, 2);
	return ntohs(r);
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
	uint16_t r;
	spi_transaction(SOCK2BANK_C(s), off, NULL, 0, (uint8_t *)&r, 2);
	return ntohs(r);
}

void w5x00_bread(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank, off, NULL, 0, pv, n);
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank, off, NULL, 0, pv, n);
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
	spi_transaction(W_WRITE, off, &n, 1, NULL, 0);
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
	spi_transaction(SOCK2BANK_C(sock) | W_WRITE, off, &n, 1, NULL, 0);
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(W_WRITE, off, (uint8_t *)&n, 2, NULL, 0);
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
	n = ntohs(n);
	spi_transaction(SOCK2BANK_C(sock) | W_WRITE, off, (uint8_t *)&n, 2, NULL, 0);
}

void w5x00_bwrite(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank|W_WRITE, off, pv, n, NULL, 0);
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
	spi_transaction(bank|W_WRITE, off, pv, n, NULL, 0);
}

void w5x00_setup(void)
{
	/* These delays are excessive but the reset seems to be very fragile */
	volatile uint32_t n = 0;
	GPOC = 1 << RESETPIN;
	while(n++ < 5000000);
	GPOS = 1 << RESETPIN;
	spi_set_clock(1, 1);
	n = 0;
	while(n++ < 5000000);
	w5x00_writecb(0, 0x80);
	n = 0;
	while(n++ < 5000000);
	w5x00_readcb(0x19);
	w5x00_readcb(0x19);
	w5x00_readcb(0x19);
	w5x00_readcb(0x1A);
	w5x00_readcb(0x1A);
}

#endif

/* vim: sw=4 ts=4 et: */
