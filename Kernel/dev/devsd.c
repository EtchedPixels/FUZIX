/*-----------------------------------------------------------------------*/
/* Fuzix SD card driver                                                  */
/* 2014-12-28 Will Sowerbutts                                            */
/* 2015-01-04 WRS updated to new blkdev API                              */
/*                                                                       */
/* Based on UZI-socz80 SD card driver, which was itself based on:        */
/*   MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2007         */
/*  (from http://www.cl.cam.ac.uk/teaching/1011/P31/lib/diskio.c)        */
/* and http://elm-chan.org/docs/mmc/mmc_e.html                           */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>
#include <blkdev.h>

static uint8_t sd_card_type[SD_DRIVE_COUNT];

/* internal functions */
static void sd_init_drive(uint8_t drive);
static int sd_spi_init(uint8_t drive);
static void sd_spi_release(uint8_t drive);
static int sd_spi_wait_ready(uint8_t drive);
static bool sd_spi_transmit_sector(uint8_t drive, void *ptr, unsigned int length);
static bool sd_spi_receive_sector(uint8_t drive, void *ptr, unsigned int length);
static int sd_send_command(uint8_t drive, unsigned char cmd, uint32_t arg);
static uint32_t sd_get_size_sectors(uint8_t drive);

static bool devsd_transfer_sector(uint8_t drive, uint32_t lba, void *buffer, bool read_notwrite)
{
    uint8_t attempt;
    bool success;

    if(!(sd_card_type[drive] & CT_BLOCK))
	lba <<= 9; /* multiply by 512 to convert block to byte address */

    for(attempt=0; attempt<8; attempt++){
	if(sd_send_command(drive, read_notwrite ? CMD17 : CMD24, lba) == 0){
	    if(read_notwrite)
		success = sd_spi_receive_sector(drive, buffer, 512);
	    else
		success = sd_spi_transmit_sector(drive, buffer, 512);
	}else
	    success = false;

	sd_spi_release(drive);

	if(success)
	    return 1;

	kputs("sd: failed, retrying.\n");
    }

    udata.u_error = EIO;
    return -1;
}

static void sd_spi_release(uint8_t drive)
{
    sd_spi_raise_cs(drive);
    sd_spi_receive_byte(drive);
}

static int sd_spi_wait_ready(uint8_t drive)
{
    unsigned char res;
    timer_t timer;

    timer = set_timer_ms(100);
    sd_spi_receive_byte(drive);
    do{
        res = sd_spi_receive_byte(drive);
        if(timer_expired(timer)){
            kputs("sd_spi_wait_ready: timeout\n");
            break;
        }
    }while ((res != 0xFF));

    return res;
}

static bool sd_spi_transmit_sector(uint8_t drive, void *ptr, unsigned int length)
{
    unsigned char reply;

    if(sd_spi_wait_ready(drive) != 0xFF)
        return false; /* failed */

    sd_spi_transmit_byte(drive, 0xFE);
    sd_spi_transmit_block(drive, ptr, length);
    sd_spi_transmit_byte(drive, 0xFF); /* dummy CRC */
    sd_spi_transmit_byte(drive, 0xFF);
    reply = sd_spi_receive_byte(drive);
    if((reply & 0x1f) != 0x05)
        return false; /* failed */
    return true; /* hooray! */
}

static bool sd_spi_receive_sector(uint8_t drive, void *ptr, unsigned int length)
{
    unsigned int timer;
    unsigned char b;

    timer = set_timer_ms(250);

    do{
        b = sd_spi_receive_byte(drive);
        if(timer_expired(timer)){
            kputs("sd_spi_receive_sector: timeout\n");
            return false;
        }
    }while(b == 0xFF);

    if(b != 0xFE)
        return false; /* failed */

    return sd_spi_receive_block(drive, ptr, length); /* returns true on success */
}

static int sd_send_command(uint8_t drive, unsigned char cmd, uint32_t arg)
{
    unsigned char n, res, *p;

    if (cmd & 0x80) {   /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        res = sd_send_command(drive, CMD55, 0);
        if (res > 1) 
            return res;
    }

    /* Select the card and wait for ready */
    sd_spi_raise_cs(drive);
    sd_spi_lower_cs(drive);
    if (sd_spi_wait_ready(drive) != 0xFF) 
        return 0xFF;

    /* Send command packet */
    sd_spi_transmit_byte(drive, cmd);                        /* Start + Command index */
#if 0
    sd_spi_transmit_byte(drive, (unsigned char)(arg >> 24)); /* Argument[31..24] */
    sd_spi_transmit_byte(drive, (unsigned char)(arg >> 16)); /* Argument[23..16] */
    sd_spi_transmit_byte(drive, (unsigned char)(arg >> 8));  /* Argument[15..8] */
    sd_spi_transmit_byte(drive, (unsigned char)arg);         /* Argument[7..0] */
#else
    /* sdcc sadly unable to figure this out for itself yet */
    p = (unsigned char *)&arg;
    sd_spi_transmit_byte(drive, p[3]);                       /* Argument[31..24] */
    sd_spi_transmit_byte(drive, p[2]);                       /* Argument[23..16] */
    sd_spi_transmit_byte(drive, p[1]);                       /* Argument[15..8] */
    sd_spi_transmit_byte(drive, p[0]);                       /* Argument[7..0] */
#endif
    /* there's only a few commands (in native mode) that need correct CRCs */
    n = 0x01;                                                /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                               /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;                               /* Valid CRC for CMD8(0x1AA) */
    sd_spi_transmit_byte(drive, n);

    /* Receive command response */
    if (cmd == CMD12) 
        sd_spi_receive_byte(drive);     /* Skip a stuff byte when stop reading */
    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do{
        res = sd_spi_receive_byte(drive);
    }while ((res & 0x80) && --n);

    return res;         /* Return with the response value */
}

/****************************************************************************/
/* Code below this point used only once, at startup, so we want it to live  */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file. This "solution" is a bit (well, very) hacky ...        */
/****************************************************************************/
static void DISCARDSEG(void) __naked { __asm .area _DISCARD __endasm; }

void devsd_init(void)
{
    uint8_t d;

    for(d=0; d<SD_DRIVE_COUNT; d++)
        sd_init_drive(d);
}

static void sd_init_drive(uint8_t drive)
{
    uint32_t sector_count;

    kprintf("SD drive %d: ", drive);
    sd_card_type[drive] = sd_spi_init(drive);

    if(!(sd_card_type[drive] & (~CT_BLOCK))){
        kprintf("no card found\n");
        return;
    }
    
    /* read and compute card size */
    sector_count = sd_get_size_sectors(drive);
    if(!sector_count){
        kputs("weird card\n");
        return;
    }

    blkdev_add(devsd_transfer_sector, drive, sector_count);
}

static int sd_spi_init(uint8_t drive)
{
    unsigned char n, cmd, card_type, ocr[4];
    timer_t timer;

    sd_spi_raise_cs(drive);

    sd_spi_clock(drive, false);
    for (n = 20; n; n--)
        sd_spi_receive_byte(drive); /* 160 dummy clocks */

    card_type = 0;
    /* Enter Idle state */
    if (sd_send_command(drive, CMD0, 0) == 1) {
        /* initialisation timeout 1 second */
        timer = set_timer_sec(1);
        if (sd_send_command(drive, CMD8, (uint32_t)0x1AA) == 1) {    /* SDHC */
            /* Get trailing return value of R7 resp */
            for (n = 0; n < 4; n++) ocr[n] = sd_spi_receive_byte(drive);
            /* The card can work at vdd range of 2.7-3.6V */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                /* Wait for leaving idle state (ACMD41 with HCS bit) */
                while(!timer_expired(timer) && sd_send_command(drive, ACMD41, (uint32_t)1 << 30));
                /* Check CCS bit in the OCR */
                if (!timer_expired(timer) && sd_send_command(drive, CMD58, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = sd_spi_receive_byte(drive);
                    card_type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;   /* SDv2 */
                }
            }
        } else { /* SDSC or MMC */
            if (sd_send_command(drive, ACMD41, 0) <= 1) {
                /* SDv1 */
                card_type = CT_SD1;
                cmd = ACMD41;
            } else {
                /* MMCv3 */
                card_type = CT_MMC;
                cmd = CMD1;
            }
            /* Wait for leaving idle state */
            while(!timer_expired(timer) && sd_send_command(drive, cmd, 0));
            /* Set R/W block length to 512 */
            if(timer_expired || sd_send_command(drive, CMD16, 512) != 0)
                card_type = 0;
        }
    }
    sd_spi_release(drive);

    if (card_type) {
        sd_spi_clock(drive, true); /* up to 20MHz should be OK */
        return card_type;
    }

    return 0; /* failed */
}

static uint32_t sd_get_size_sectors(uint8_t drive)
{
    unsigned char csd[16], n;
    uint32_t sectors = 0;

    if(sd_send_command(drive, CMD9, 0) == 0 && sd_spi_receive_sector(drive, csd, 16)){
        if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
            sectors = ((uint32_t)csd[9] + (uint32_t)((unsigned int)csd[8] << 8) + 1) << 10;
        } else {					/* SDC ver 1.XX or MMC*/
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            sectors = (csd[8] >> 6) + ((unsigned int)csd[7] << 2) + ((unsigned int)(csd[6] & 3) << 10) + 1;
            sectors = sectors << (n - 9);
        }
    }
    sd_spi_release(drive);
    return sectors;
}
