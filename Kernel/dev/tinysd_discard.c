/*
 *	Optional SD card initialization for tinysd
 *
 *	Does card initialzation and probing. Returns a card type.
 */

#include <kernel.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <printf.h>

#ifdef CONFIG_TD_SD

static uint8_t cmd0[6]	= { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
static uint8_t cmd1[6]	= { 0x41, 0x00, 0x00, 0x00, 0x00, 0x01 };
static uint8_t cmd8[6]	= { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x87 };
static uint8_t cmd16[6] = { 0x50, 0x00, 0x00, 0x02, 0x00, 0x01 };
static uint8_t cmd55[6] = { 0x77, 0x00, 0x00, 0x00, 0x00, 0x01 };
static uint8_t cmd58[6] = { 0x7A, 0x00, 0x00, 0x00, 0x00, 0x01 };
static uint8_t acmd41_0[6] = { 0x69, 0x00, 0x00, 0x00, 0x00, 0x01 };
static uint8_t acmd41[6] = {0x69, 0x40, 0x00, 0x00, 0x00, 0x01 };

static uint8_t sdbuf[4];

static uint8_t sendcmd(uint8_t *cmd)
{
    uint8_t n = 0;
    uint8_t r;
    sd_spi_raise_cs();
    sd_spi_receive_byte();
    sd_spi_lower_cs();
    if (*cmd != 0x40) {
        while(++n && sd_spi_receive_byte() != 0xFF);
        if (n == 0)
            return 0xFF;
    }
    n = 0;
    while(++n <= 6)
        sd_spi_transmit_byte(*cmd++);
    sd_spi_receive_byte();
    n = 0xA0;
    while(++n) {
        r = sd_spi_receive_byte();
        if (!(r & 0x80))
            break;
    }
    if (n == 0)
        return 0xFF;
    return r;
}

static int sendacmd(uint8_t *cmd)
{
    int err;
    err = sendcmd(cmd55);
    if (err > 1)
        return err;
    return sendcmd(cmd);
}

static void sd_get4(void)
{
    uint8_t *p = sdbuf;
    uint_fast8_t n = 0;
    while(++n <= 4)
        *p++ = sd_spi_receive_byte();
}

static uint8_t sdhc_init(void)
{
    uint8_t n = 0;
    sd_get4();
    if (sdbuf[2] != 0x01 || sdbuf[3] != 0xAA)
        return CT_NONE;
    while(++n && sendacmd(acmd41));
    if (n == 0)
        return CT_NONE;
    if (sendcmd(cmd58))
        return CT_NONE;
    sd_get4();
    sd_spi_fast();
    if (sdbuf[0] & 0x40)
        return CT_SD2 | CT_BLOCK;
    return CT_SD2;
}

static uint8_t mmc_init(void)
{
    uint8_t n = 0;
    while(++n && sendcmd(cmd1));
    if (n == 0)
        return CT_NONE;
    if (sendcmd(cmd16))
        return CT_NONE;
    sd_spi_fast();
    return CT_MMC;
}

static uint8_t sd_try_init(void)
{
    uint_fast8_t n = 0;
    sd_spi_raise_cs();
    sd_spi_receive_byte();
    while(++n <= 8)
        sd_spi_receive_byte();
    if (sendcmd(cmd0) != 1)
        return CT_NONE;
    if (sendcmd(cmd8) == 1)
        return sdhc_init();
    if (sendacmd(acmd41_0) > 1)
        return mmc_init();
    n = 0;
    while(++n && sendacmd(acmd41_0));
    /* Timed out */
    if (n == 0)
        return CT_NONE;
    sendcmd(cmd16);
    return CT_SD1;
}

uint8_t sd_init(uint_fast8_t unit)
{
    uint_fast8_t n = 0;
    uint8_t r;

    tinysd_busy = 1;
    tinysd_unit = unit;
    sd_spi_slow();

    do {
        r = sd_try_init();
    } while(r == CT_NONE && ++n < 8);

    tinysd_busy = 0;
    return r;
}

#ifdef TD_SD_NUM

void sd_probe(void)
{
    uint_fast8_t n = 0;
    int r;
    uint_fast8_t t;

    for (n = 0; n < TD_SD_NUM; n++) {
        t = sd_init(n);
        if (t != CT_NONE) {
            /* Partition is block 0 so we can worry about shifts later */
            r = td_register(sd_xfer, 1);
            if (r < 0)
                continue;
            if (!(t & CT_BLOCK))
                sd_shift[r] = 9;
            sd_dev[r] = n;
        }
    }
    kputchar('\r');
}
#endif

#endif
