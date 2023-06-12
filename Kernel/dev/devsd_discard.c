/*-----------------------------------------------------------------------*/
/* Fuzix SD card driver                                                  */
/* 2014-12-28 Will Sowerbutts                                            */
/* 2015-01-04 WRS updated to new blkdev API                              */
/* 2015-01-25 WRS updated to newer blkdev API                            */
/*                                                                       */
/* Based on UZI-socz80 SD card driver, which was itself based on:        */
/*   MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2007         */
/*  (from http://www.cl.cam.ac.uk/teaching/1011/P31/lib/diskio.c)        */
/* and http://elm-chan.org/docs/mmc/mmc_e.html                           */
/*-----------------------------------------------------------------------*/

#define _SD_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>
#include <blkdev.h>

#ifdef CONFIG_SD

/****************************************************************************/
/* Code in this file is used only once, at startup, so we want it to live   */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file.                                                        */
/****************************************************************************/

void devsd_init(void)
{
    uint8_t i;
    for(i = 0; i < SD_DRIVE_COUNT; i++)
        sd_init_drive(i);
}

void sd_init_drive(uint8_t drive)
{
    blkdev_t *blk;
    unsigned char csd[16], n;
    uint_fast8_t card_type;

    sd_drive = drive;
    kprintf("SD drive %d: ", sd_drive);
    card_type = sd_spi_init();

    if(!(card_type & (~CT_BLOCK))){
        kprintf("no card found\n");
        return;
    }

    blk = blkdev_alloc();
    if(!blk)
        return;

    blk->transfer = devsd_transfer_sector;
    blk->driver_data = (sd_drive & DRIVE_NR_MASK) | card_type;
    
    /* read and compute card size */
    if(sd_send_command(CMD9, 0) == 0 && sd_spi_wait(false) == 0xFE){
        for(n=0; n<16; n++)
            csd[n] = sd_spi_receive_byte();
        if ((csd[0] >> 6) == 1) {
            /* SDC ver 2.00 */
            blk->drive_lba_count = ((uint32_t)csd[9] 
                                   + (uint32_t)((unsigned int)csd[8] << 8) + 1) << 10;
        } else {
            /* SDC ver 1.XX or MMC*/
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            blk->drive_lba_count = (csd[8] >> 6) + ((unsigned int)csd[7] << 2) 
                                   + ((unsigned int)(csd[6] & 3) << 10) + 1;
            blk->drive_lba_count <<= (n-9);
        }
    }
    sd_spi_release();

    blkdev_scan(blk, 0);
}

int sd_spi_init(void)
{
    uint_fast8_t n, cmd, card_type;
    unsigned char ocr[4];
    timer_t timer;

	/* Initialising SD cards is pretty horrible; they aren't sane SPI devices.
	 * There's a good flowchart of the state machine and some docs here:
	 *
	 * http://elm-chan.org/docs/mmc/ima/sdinit.png
	 * http://elm-chan.org/docs/mmc/mmc_e.html
	 */

    sd_spi_raise_cs();

    sd_spi_clock(false);
    for (n = 20; n; n--)
        sd_spi_receive_byte(); /* send dummy clocks -- at least 80 required; we send 160 */

    card_type = CT_NONE;

    /* Enter Idle state */
    if (sd_send_command(CMD0, 0) == 1) {
        /* initialisation timeout 2 seconds */
        timer = set_timer_sec(2);
        if (sd_send_command(CMD8, (uint32_t)0x1AA) == 1) {    /* SDHC */
            /* Get trailing return value of R7 resp */
            for (n = 0; n < 4; n++) ocr[n] = sd_spi_receive_byte();
            /* The card can work at vdd range of 2.7-3.6V */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                /* Wait for leaving idle state (ACMD41 with HCS bit) */
                while(!timer_expired(timer) && sd_send_command(ACMD41, (uint32_t)1 << 30));
                /* Check CCS bit in the OCR */
                if (!timer_expired(timer) && sd_send_command(CMD58, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = sd_spi_receive_byte();
                    card_type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;   /* SDv2 */
                }
            }
        } else { /* SDSC or MMC */
            if (sd_send_command(ACMD41, 0) <= 1) {
                /* SDv1 */
                card_type = CT_SD1;
                cmd = ACMD41;
            } else {
                /* MMCv3 */
                card_type = CT_MMC;
                cmd = CMD1;
            }
            /* Wait for leaving idle state */
            while(!timer_expired(timer) && sd_send_command(cmd, 0));
            /* Set R/W block length to 512 */
            if(timer_expired(timer) || sd_send_command(CMD16, 512) != 0)
                card_type = CT_NONE;
        }
    }
    sd_spi_release();

    if (card_type) {
        sd_spi_clock(true); /* up to 20MHz should be OK */
        return card_type;
    }

    return CT_NONE; /* failed */
}

#endif
