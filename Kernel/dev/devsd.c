/*-----------------------------------------------------------------------*/
/* Fuzix SD card driver                                                  */
/* 2014-12-28 Will Sowerbutts                                            */
/*                                                                       */
/* Based on UZI-socz80 SD card driver, which was itself based on:        */
/*   MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2007         */
/*  (from http://www.cl.cam.ac.uk/teaching/1011/P31/lib/diskio.c)        */
/* and http://elm-chan.org/docs/mmc/mmc_e.html                           */
/*-----------------------------------------------------------------------*/

/* Minor numbers: See comments in devide.c, exactly the same scheme is used here. */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>
#include <mbr.h>
#include "config.h"

#define MAX_SLICES 63

/* keep track of current card type, UZI partition offset */
static uint8_t  sd_drive_present; /* bitmap */
static uint8_t  sd_card_type[SD_DRIVE_COUNT];
static uint32_t sd_partition_start[SD_DRIVE_COUNT];
static uint8_t  sd_slice_count[SD_DRIVE_COUNT];

/* internal functions */
static void sd_init_drive(uint8_t drive);
static int sd_readwrite(uint8_t minor, uint8_t rawflag, bool do_write);
static int sd_spi_init(uint8_t drive);
static void sd_spi_release(uint8_t drive);
static int sd_spi_wait_ready(uint8_t drive);
static bool sd_spi_transmit_sector(uint8_t drive, void *ptr, unsigned int length);
static bool sd_spi_receive_sector(uint8_t drive, void *ptr, unsigned int length);
static int sd_send_command(uint8_t drive, unsigned char cmd, uint32_t arg);
static uint32_t sd_get_size_sectors(uint8_t drive);
static bool sd_read_sector(uint8_t drive, void *ptr, uint32_t lba);
static bool sd_write_sector(uint8_t drive, void *ptr, uint32_t lba);

int devsd_open(uint8_t minor, uint16_t flags)
{
    uint8_t drive;
    flags; /* not used */

    drive = minor >> 6;
    minor = minor & 0x3F;

    if(sd_drive_present & (1 << drive) && (minor == 0 || minor < sd_slice_count[drive]))
        return 0;

    udata.u_error = ENODEV;
    return -1;
}

int devsd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return sd_readwrite(minor, rawflag, false);
}

int devsd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return sd_readwrite(minor, rawflag, true);
}

void devsd_init(void)
{
    uint8_t d;

    sd_drive_present = 0;
    for(d=0; d<SD_DRIVE_COUNT; d++)
        sd_init_drive(d);
}

static int sd_readwrite(uint8_t minor, uint8_t rawflag, bool do_write)
{
    uint32_t lba;
    int attempt;
    void *target;
    uint8_t drive;

    drive = minor >> 6;
    minor = minor & 0x3F;

    if(rawflag == 0){
        target = udata.u_buf->bf_data;

        for(attempt=0; attempt<5; attempt++){
            lba = udata.u_buf->bf_blk;

            /* minor 0 is the whole drive, without translation */
            if(minor > 0){
                lba += sd_partition_start[drive];
                lba += ((unsigned long)(minor-1) << SLICE_SIZE_LOG2_SECTORS);
            }
            if(do_write){
                if(sd_write_sector(drive, udata.u_buf->bf_data, lba))
                    return 1;
            }else{
                if(sd_read_sector(drive, udata.u_buf->bf_data, lba))
                    return 1;
            }
            kputs("sd: failed, retrying.\n");
        }
    }

    udata.u_error = EIO;
    return -1;
}

static void sd_init_drive(uint8_t drive)
{
    uint32_t sector_count;
    unsigned char *sector;

    sd_partition_start[drive] = 0;
    sd_slice_count[drive] = 0;

    kprintf("sd%c: ", 'a'+drive);
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

    /* read partition table, locate the UZI partition */
    sector = (unsigned char*)tmpbuf();

    if(!sd_read_sector(drive, sector, 0)){
        kprintf("failed to read partition table\n");
        goto failout;
    }

    /* if we get this far the drive is probably ok */
    sd_drive_present |= (1 << drive);

    /* look for our partition */
    parse_partition_table(sector, &sd_partition_start[drive], &sd_slice_count[drive], MAX_SLICES);

failout:
    brelse((bufptr)sector);
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

static bool sd_read_sector(uint8_t drive, void *ptr, uint32_t lba)
{
    bool r = false;

    if(sd_card_type[drive] != CT_NONE){
        if(!(sd_card_type[drive] & CT_BLOCK))
            lba = lba << 9; /* multiply by 512 to get byte address */

        if(sd_send_command(drive, CMD17, lba) == 0)
            r = sd_spi_receive_sector(drive, ptr, 512);

        sd_spi_release(drive);
    }

    return r;
}

static bool sd_write_sector(uint8_t drive, void *ptr, uint32_t lba)
{
    bool r = false;

    if(sd_card_type[drive] != CT_NONE){
        if(!(sd_card_type[drive] & CT_BLOCK))
            lba = lba << 9; /* multiply by 512 to get byte address */

        if(sd_send_command(drive, CMD24, lba) == 0)
            r = sd_spi_transmit_sector(drive, ptr, 512);

        sd_spi_release(drive);
    }

    return r;
}
