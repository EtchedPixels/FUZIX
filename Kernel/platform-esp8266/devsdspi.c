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

#define CSPIN 4			/* also change GPF5 and GPC5 below */

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
	GPF4 = GPFFS(GPFFS_GPIO(CSPIN));	/* CS */
	GPF14 = GPFFS(GPFFS_BUS(14));	/* CLK */
	GPF13 = GPFFS(GPFFS_BUS(13));	/* MTCK, MOSI */
	GPF12 = GPFFS(GPFFS_BUS(12));	/* MTDI, MISO */
	GPC4 = 0;		/* GPIO_DATA, normal driver, interrupt disabled, wakeup disabled */
	GPES = 1 << CSPIN;

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

void sd_spi_clock(bool go_fast)
{
	setFrequency(go_fast ? 4000000 : 250000);
}

void sd_spi_raise_cs(void)
{
	GPOS = 1 << CSPIN;
}

void sd_spi_lower_cs(void)
{
	GPOC = 1 << CSPIN;
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
 *	TODO: this only works because aligned memcpy is optimized into a 32bit store. It also only
 *	works because we don't have direct I/O enabled, which for a CPU to device speed ratio this big
 *	may be the right thing anyway.
 */
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

		memcpy(dptr, (const void *) &SPI1W0, 64);
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
		memcpy((void *) &SPI1W0, sptr, 64);
		SPI1CMD = SPIBUSY;

		while (SPI1CMD & SPIBUSY);

		sptr += 64;
	}

	return 0;
}

/* vim: sw=4 ts=4 et: */
