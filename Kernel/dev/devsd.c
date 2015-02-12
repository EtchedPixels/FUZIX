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

uint8_t devsd_transfer_sector(void)
{
    uint8_t attempt, drive;
    bool success;

    drive = blk_op.blkdev->driver_data & DRIVE_NR_MASK;

    for(attempt=0; attempt<8; attempt++){
	if(sd_send_command(drive, blk_op.is_read ? CMD17 : CMD24, 
                    /* for byte addressed cards, shift LBA to convert to byte address */
                    (blk_op.blkdev->driver_data & CT_BLOCK) ? blk_op.lba : (blk_op.lba << 9)
                    ) == 0){
	    if(blk_op.is_read){
                success = (sd_spi_wait(drive, false) == 0xFE);
                if(success)
                    sd_spi_receive_sector(drive);
            }else{
                success = false;
                if(sd_spi_wait(drive, true) == 0xFF){
                    sd_spi_transmit_byte(drive, 0xFE);
                    sd_spi_transmit_sector(drive);
                    sd_spi_transmit_byte(drive, 0xFF); /* dummy CRC */
                    sd_spi_transmit_byte(drive, 0xFF);
                    success = ((sd_spi_wait(drive, false) & 0x1F) == 0x05);
                }
            }
	}else
	    success = false;

	sd_spi_release(drive);

	if(success)
	    return 1;

	kputs("sd: failed, retrying.\n");
    }

    udata.u_error = EIO;
    return 0;
}

void sd_spi_release(uint8_t drive)
{
    sd_spi_raise_cs(drive);
    sd_spi_receive_byte(drive);
}

uint8_t sd_spi_wait(uint8_t drive, bool want_ff)
{
    unsigned int timer;
    unsigned char b;

    timer = set_timer_ms(500);

    while(true){
        b = sd_spi_receive_byte(drive);
        if(want_ff){
            if(b == 0xFF)
                break;
        }else{
            if(b != 0xFF)
                break;
        }
        if(timer_expired(timer)){
            kputs("sd: timeout\n");
            break;
        }
    }

    return b;
}

int sd_send_command(uint8_t drive, unsigned char cmd, uint32_t arg)
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
    if(sd_spi_wait(drive, true) != 0xFF)
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
    p = ((unsigned char *)&arg)+3;
    sd_spi_transmit_byte(drive, *(p--));                     /* Argument[31..24] */
    sd_spi_transmit_byte(drive, *(p--));                     /* Argument[23..16] */
    sd_spi_transmit_byte(drive, *(p--));                     /* Argument[15..8] */
    sd_spi_transmit_byte(drive, *p);                         /* Argument[7..0] */
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
