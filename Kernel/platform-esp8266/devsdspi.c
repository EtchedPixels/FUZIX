#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <blkdev.h>
#include <eagle_soc.h>
#include <spi_register.h>
#include "dev/devsd.h"
#include "esp8266_peri.h"
#include "globals.h"
#include "config.h"

typedef union
{
    uint32_t regValue;
    struct
    {
            unsigned regL: 6;
            unsigned regH: 6;
            unsigned n: 6;
            unsigned pre: 13;
            unsigned regEQU: 1;
    };
} spiclock_t;

void sd_rawinit(void)
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); /* CS */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); /* MOSI */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); /* MISO */
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); /* CLK */
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, 1<<15);
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U);
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U);

    SPI1C = 0;
    SPI1U = 0;
    SPI1P = 0;
    SPI1U1 = 0;
    SPI1C1 = 0;
	SPI1S = 0;
}

static uint32_t calculate_freq(spiclock_t* reg)
{
    return ((PERIPHERAL_CLOCK*1000000)
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

#if 0
static int32_t abs(int32_t i)
{
    if (i < 0)
        return -i;
    return i;
}
#endif

/* This code is borrowed heavily from the Arduino code at
 * https://github.com/esp8266/Arduino/blob/master/libraries/SPI/SPI.cpp
 * I don't understand how it works --- the clock computation appears
 * to be invalid. But it does. */

static void setFrequency(uint32_t freq) {
    if(freq >= (PERIPHERAL_CLOCK*1000000))
    {
        /* magic number to set spi sysclock bit (see below.) */
        set_clock(0x80000000);
        return;
    }

    const spiclock_t minFreqReg = { 0x7FFFF020 };
    uint32_t minFreq = calculate_freq((spiclock_t*) &minFreqReg);
    if(freq < minFreq)
    {
        // use minimum possible clock regardless
        set_clock(minFreqReg.regValue);
        return;
    }

    uint8_t calN = 1;

    spiclock_t bestReg = { 0 };
    int32_t bestFreq = 0;

    // aka 0x3F, aka 63, max for n:6
    const uint8_t nMax = (1 << 6) - 1;

    // aka 0x1fff, aka 8191, max for pre:13
    const int32_t preMax = (1 << 13) - 1;

    // find the best match for the next 63 iterations
    while(calN <= nMax) {

        spiclock_t reg = { 0 };
        int32_t calFreq;
        int32_t calPre;
        int8_t calPreVari = -2;

        reg.n = calN;

        while(calPreVari++ <= 1)
        {
            calPre = (((ESP8266_CLOCK / (reg.n + 1)) / freq) - 1) + calPreVari;
            if(calPre > preMax) {
                reg.pre = preMax;
            } else if(calPre <= 0) {
                reg.pre = 0;
            } else {
                reg.pre = calPre;
            }

            reg.regL = ((reg.n + 1) / 2);
            // reg.regH = (reg.n - reg.regL);

            // test calculation
            calFreq = calculate_freq(&reg);
            //kprintf("-----[%p][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d = %d\n", reg.regValue, freq, reg.regEQU, reg.pre, reg.n, reg.regH, reg.regL, calFreq);

            if (calFreq == (int32_t)(freq))
            {
                // accurate match use it!
                memcpy(&bestReg, &reg, sizeof(bestReg));
                break;
            }
            else if (calFreq < (int32_t)(freq))
            {
                // never go over the requested frequency
                int32_t cal = abs((int32_t)(freq) - calFreq);
                int32_t best = abs((int32_t)(freq) - bestFreq);
                if(cal < best)
                {
                    bestFreq = calFreq;
                    memcpy(&bestReg, &reg, sizeof(bestReg));
                }
            }
        }
        if (calFreq == (int32_t)(freq))
            break;

        calN++;
    }

    //kprintf("[%p][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d\t - Real Frequency: %d\n", bestReg.regValue, freq, bestReg.regEQU, bestReg.pre, bestReg.n, bestReg.regH, bestReg.regL, calculate_freq(&bestReg));

    set_clock(bestReg.regValue);
}

void sd_spi_clock(bool go_fast)
{
    setFrequency(go_fast ? 8000000 : 250000);
}

void sd_spi_raise_cs(void)
{
	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<15);
}

void sd_spi_lower_cs(void)
{
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<15);
}

static uint_fast8_t send_recv(uint_fast8_t data)
{
	while (SPI1CMD & SPIBUSY)
		;

    SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
    SPI1U = SPIUMOSI | SPIUDUPLEX;
    SPI1W0 = data;
	SPI1CMD = SPIBUSY;

	while (SPI1CMD & SPIBUSY)
		;

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

bool sd_spi_receive_sector(void)
{
    uint8_t* dptr = (uint8_t*)blk_op.addr;
    if ((uint32_t)dptr & 3)
        panic("unaligned read");
    for (int chunk = 0; chunk < 8; chunk++)
    {
        while (SPI1CMD & SPIBUSY)
            ;

        SPI1U1 = (511 << SPILMISO);
        SPI1U = SPIUMISO;
        SPI1CMD = SPIBUSY;

        while (SPI1CMD & SPIBUSY)
            ;

        memcpy(dptr, &SPI1W0, 64);
        dptr += 64;
    }

	return 0;
}

bool sd_spi_transmit_sector(void)
{
    uint8_t* sptr = (uint8_t*)blk_op.addr;
    if ((uint32_t)sptr & 3)
        panic("unaligned write");
    for (int chunk = 0; chunk < 8; chunk++)
    {
        while (SPI1CMD & SPIBUSY)
            ;

        SPI1U1 = (511 << SPILMOSI);
        SPI1U = SPIUMOSI;
        memcpy((void*)&SPI1W0, sptr, 64);
        SPI1CMD = SPIBUSY;

        while (SPI1CMD & SPIBUSY)
            ;

        sptr += 64;
    }

	return 0;
}

/* vim: sw=4 ts=4 et: */

