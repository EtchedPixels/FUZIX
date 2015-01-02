/*-----------------------------------------------------------------------*/
/* socz80 SD card driver                                                 */
/* Based on:                                                             */
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

/* keep track of current card type, UZI partition offset */
static int sd_card_type;
static unsigned long sd_first_uzi_sector;

/* used to track current transfer in progress */
static int sd_blockdev_count;
static char *sd_dptr;
static int sd_dlen;
static uint16_t sd_blocks;
static unsigned long sd_next_block;

void sd_setup(uint8_t minor, uint8_t rawflag)
{
    blkno_t block;

    if(rawflag){
        sd_dlen = udata.u_count;
        sd_dptr = udata.u_base;
        block = udata.u_offset >> 9;
        sd_blocks = sd_dlen >> 9;
    }else{
        sd_dlen = 512;
        sd_dptr = udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
        sd_blocks = 1;
    }

    if(sd_blocks != 1)
        panic("sd: unexpected block count");

    sd_next_block = sd_first_uzi_sector + /* start of our partition */
        (((unsigned long)minor) << UZI_BLOCKDEV_SIZE_LOG2_SECTORS) + /* start of this minor device */
        ((unsigned long)block); /* requested sector */
}

int sd_open(uint8_t minor, uint16_t flag)
{
    flag;
    if(minor < sd_blockdev_count){
        return 0;
    } else {
        udata.u_error = EIO;
        return -1;
    }
}

int sd_readwrite(uint8_t minor, uint8_t rawflag, bool do_write)
{
    int attempt;
    for(attempt=0; attempt<5; attempt++){
        sd_setup(minor, rawflag);
        if(do_write){
            if(sd_write_sector(sd_dptr, sd_next_block))
                return sd_blocks;
        }else{
            if(sd_read_sector(sd_dptr, sd_next_block))
                return sd_blocks;
        }
        kputs("sd: failed, resetting.\n");
        if(sd_init())
            break;
    }
    return -1;
}

int sd_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return sd_readwrite(minor, rawflag, false);
}

int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag;
    return sd_readwrite(minor, rawflag, true);
}

static const char cardname[] = "     MMC\0\0SDv1\0?>???SDv2\0";

/* Stop sdcc inlining a ton of crap each time */
static unsigned long shift11(unsigned long p)
{
    return (p >> 11);
}

void sd_print_partition(unsigned char *p)
{
    long size;
    if(!*p)
        return;
        
    size = *(unsigned long *)(p + 4);
    
    kprintf("sd: partition type 0x%x, offset %uMB, length %uMB (%s)\n",
             *p,
             (unsigned int)shift11(*(unsigned long *)(p + 4)),
             (unsigned int)shift11(size),
             *p != UZI_PARTITION_TYPE ? "ignored" : 
             (sd_blockdev_count == 0 ? "UZI" : "unused"));

    /* We will use the first partition with the appropriate type */
    if(*p == UZI_PARTITION_TYPE && sd_blockdev_count == 0){
        sd_first_uzi_sector = *((unsigned long*)&p[4]);
        /* 16 bit shift */
        sd_blockdev_count = size >> UZI_BLOCKDEV_SIZE_LOG2_SECTORS;
        /* Allow a smaller than 32MB final 'device' */
        if (size & 0xFFFF)
            sd_blockdev_count++;
    }
}

int sd_init(void)
{
    int n;
    unsigned long sector_count;
    unsigned char *sector;
    unsigned char *p; /* into sector */

    sd_first_uzi_sector = 0;
    sd_blockdev_count = 0;

    kputs("sd: Probing ... ");

    if (!(sd_card_type = sd_spi_init())) {
        kputs("No card found\n");
        return -1;
    }
    
    /* read and compute card size */
    sector_count = sd_get_size_sectors();
    if(!sector_count){
        kputs("Weird card\n");
        return -1;
    }
    n = shift11(sector_count);

    kprintf("Found %s card (%dMB, b%s addressed)\n", 
                cardname + 5 * (sd_card_type & ~CT_BLOCK),
                n,
                (sd_card_type & CT_BLOCK) ? "lock" : "yte");

    /* read partition table, locate the UZI partition */
    sector = (unsigned char*)tmpbuf();
    if(!sd_read_sector(sector, 0)){
        kputs("sd: Failed to read partition table\n");
    }else{
        p = sector + 510;
        if(*p != 0x55 || p[1] != 0xAA){ /* check for presence of MBR boot signature */
            kputs("sd: Cannot find MBR partition table\n");
        }else{
            p = sector + 0x1BE + 4;
            for(n = 4; n > 0; n--) {
                sd_print_partition(p);
                p+=16;
            }
            if(sd_blockdev_count == 0){
                kprintf("sd: No UZI partition (type 0x%x) found\n", UZI_PARTITION_TYPE);
            }
        }
    }

    if(sd_blockdev_count > NUM_DEV_SD)
        sd_blockdev_count = NUM_DEV_SD;

    if(sd_blockdev_count)
        kprintf("sd: %d block devices, max 32MB each\n", sd_blockdev_count);

    brelse((bufptr)sector);

    return 0; /* success */
}

int sd_spi_init(void)
{
    unsigned char n, cmd, card_type, ocr[4];
    timer_t timer;

    sd_spi_mode0();
    sd_spi_raise_cs();

    sd_spi_clock(255); /* 250kHz */
    for (n = 20; n; n--)
        sd_spi_receive_byte(); /* 160 dummy clocks */

    card_type = 0;
    /* Enter Idle state */
    if (sd_send_command(CMD0, 0) == 1) {
        /* initialisation timeout 1 second */
        timer = set_timer_sec(1);
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
            if (sd_send_command(ACMD41, 0) <= 1)    {
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
            if(timer_expired(timer) || sd_send_command(CMD16, (uint32_t)512) != 0)
                card_type = 0;
        }
    }
    sd_spi_release();

    if (card_type) {
        if(card_type == CT_MMC)
            sd_spi_clock(3); /* 16MHz (can do up to 20MHz with MMC) */
        else
            sd_spi_clock(2); /* 21MHz (can do up to 25MHz with SD) */
        return card_type;
    }

    return 0; /* failed */
}

void sd_spi_release(void)
{
    sd_spi_raise_cs();
    sd_spi_receive_byte();
}

int sd_spi_wait_ready(void)
{
    unsigned char res;
    timer_t timer;

    timer = set_timer_ms(100); /* 100ms */
    sd_spi_receive_byte();
    do{
        res = sd_spi_receive_byte();
    }while ((res != 0xFF) && !timer_expired(timer));

    return res;
}

int sd_spi_transmit_block(void *ptr, int length)
{
    unsigned char reply;

    if(sd_spi_wait_ready() != 0xFF)
        return 0; /* failed */

    sd_spi_transmit_byte(0xFE);
    sd_spi_transmit_from_memory(ptr, length);
    sd_spi_transmit_byte(0xFF); /* dummy CRC */
    sd_spi_transmit_byte(0xFF);
    reply = sd_spi_receive_byte();
    if((reply & 0x1f) != 0x05)
        return 0; /* failed */
    return 1; /* hooray! */
}

int sd_spi_receive_block(void *ptr, int length)
{
    unsigned int timer;
    unsigned char b;

    timer = set_timer_ms(200); /* 200ms */

    do{
        b = sd_spi_receive_byte();
    }while(b == 0xFF && !timer_expired(timer));
    if(b != 0xFE)
        return 0; /* failed */

    return sd_spi_receive_to_memory(ptr, length); /* returns nonzero */
}

int sd_send_command(unsigned char cmd, uint32_t arg)
{
    unsigned char n, res;

    if (cmd & 0x80) {   /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        res = sd_send_command(CMD55, 0);
        if (res > 1) 
            return res;
    }

    /* Select the card and wait for ready */
    sd_spi_raise_cs();
    sd_spi_lower_cs();
    if (sd_spi_wait_ready() != 0xFF) 
        return 0xFF;

    /* Send command packet */
    sd_spi_transmit_byte(cmd);                               /* Start + Command index */
    sd_spi_transmit_byte((unsigned char)(arg >> 24));        /* Argument[31..24] */
    sd_spi_transmit_byte((unsigned char)(arg >> 16));        /* Argument[23..16] */
    sd_spi_transmit_byte((unsigned char)(arg >> 8));         /* Argument[15..8] */
    sd_spi_transmit_byte((unsigned char)arg);                /* Argument[7..0] */
    /* there's only a few commands (in native mode) that need correct CRCs */
    n = 0x01;                                                /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                               /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;                               /* Valid CRC for CMD8(0x1AA) */
    sd_spi_transmit_byte(n);

    /* Receive command response */
    if (cmd == CMD12) 
        sd_spi_receive_byte();          /* Skip a stuff byte when stop reading */
    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do{
        res = sd_spi_receive_byte();
    }while ((res & 0x80) && --n);

    return res;         /* Return with the response value */
}

unsigned long sd_get_size_sectors(void)
{
    unsigned char csd[16], n;
    unsigned long sectors = 0;

    if(sd_send_command(CMD9, 0) == 0 && sd_spi_receive_block(csd, 16)){
        if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
            sectors = (csd[9] + ((unsigned int)csd[8] << 8) + 1) << 10;
        } else {					/* SDC ver 1.XX or MMC*/
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            sectors = (csd[8] >> 6) + ((unsigned int)csd[7] << 2) + ((unsigned int)(csd[6] & 3) << 10) + 1;
            sectors = sectors << (n - 9);
        }
    }
    sd_spi_release();
    return sectors;
}

int sd_read_sector(void *ptr, unsigned long lba)
{
    int r = 0;

    if(sd_card_type == CT_NONE)
        return r;

    if(!(sd_card_type & CT_BLOCK))
        lba = lba << 9; /* multiply by 512 to get byte address */

    if(sd_send_command(CMD17, lba) == 0 && sd_spi_receive_block(ptr, 512)){
        r = -1;
    }

    sd_spi_release();

    return r;
}

int sd_write_sector(void *ptr, unsigned long lba)
{
    int r = 0;

    if(sd_card_type == CT_NONE)
        return r;

    if(!(sd_card_type & CT_BLOCK))
        lba = lba << 9; /* multiply by 512 to get byte address */

    if(sd_send_command(CMD24, lba) == 0 && sd_spi_transmit_block(ptr, 512)){
        r = -1;
    }

    sd_spi_release();

    return r;
}
