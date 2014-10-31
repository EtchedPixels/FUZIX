#ifndef __DEVSD_DOT_H__
#define __DEVSD_DOT_H__

#define UZI_PARTITION_TYPE 0x5A    /* ASCII "Z", Wikipedia suggests this partition type is not widely used */
#define UZI_BLOCKDEV_SIZE_LOG2_SECTORS 16 /* Each device is 2^16 sectors ie 2^16 * 2^9 = 2^25 = 32MB */

/* public interface */
int sd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int sd_init(void);
int sd_open(uint8_t minor, uint16_t flag);

/* private interface */
int sd_spi_init(void);
int sd_read_sector(void *ptr, unsigned long lba);
int sd_write_sector(void *ptr, unsigned long lba);
unsigned long sd_get_size_sectors(void);

/* internal functions */
void sd_spi_clock(int divisor); 
/* 
   sd_clock sets SPI bus to 64.0 / (1+divisor) MHz 
     0=64MHz, 1=32MHz, 2=21.33MHz, 3=16MHz, 4=12.8MHz, 
     5=10.66MHz, 6=9.14MHz, 7=8MHz, 15=4MHz, etc 
     255 = 0.25MHz, about right for MMC/SPI initialisation
*/

void sd_spi_mode0(void);
void sd_spi_raise_cs(void);
void sd_spi_lower_cs(void);
void sd_spi_release(void);
int sd_spi_wait_ready(void);
void sd_spi_transmit_byte(unsigned char byte);
unsigned char sd_spi_receive_byte(void);
int sd_spi_receive_block(void *ptr, int length); /* waits for card ready then calls sd_spi_receive_to_memory */
int sd_spi_transmit_block(void *ptr, int length); /* waits for card ready then calls sd_spi_receive_to_memory */
int sd_spi_receive_to_memory(void *ptr, int length);
int sd_spi_transmit_from_memory(void *ptr, int length);
int sd_send_command(unsigned char cmd, uint32_t arg);

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)    /* GO_IDLE_STATE */
#define CMD1    (0x40+1)    /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)    /* SEND_IF_COND */
#define CMD9    (0x40+9)    /* SEND_CSD */
#define CMD10   (0x40+10)   /* SEND_CID */
#define CMD12   (0x40+12)   /* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)   /* SD_STATUS (SDC) */
#define CMD16   (0x40+16)   /* SET_BLOCKLEN */
#define CMD17   (0x40+17)   /* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)   /* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)   /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)   /* WRITE_BLOCK */
#define CMD25   (0x40+25)   /* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)   /* APP_CMD */
#define CMD58   (0x40+58)   /* READ_OCR */

#define CT_NONE 0x00
#define CT_MMC 0x01
#define CT_SD1 0x02
#define CT_SD2 0x04
#define CT_SDC (CT_SD1|CT_SD2)
#define CT_BLOCK 0x08

#endif /* __DEVSD_DOT_H__ */
