#ifndef __DEVSD_DOT_H__
#define __DEVSD_DOT_H__

/* SD Configuration (in config.h)

   Define DEVICE_SD if SD hardware is present on your platform.

   Define SD_DRIVE_COUNT to the number of SD cards your hardware
   supports (range 1--4).

   Provide the platform-specific SPI functions listed below to
   drive the SPI bus on your hardware, for an example of how to
   do this see platform-n8vem-mark4/devsdspi.c
*/


/* public interface */
void devsd_init(void);

/* platform-specific SPI functions */
void sd_spi_clock(uint8_t drive, bool go_fast); 
void sd_spi_raise_cs(uint8_t drive);
void sd_spi_lower_cs(uint8_t drive);
void sd_spi_transmit_byte(uint8_t drive, uint8_t byte);
uint8_t sd_spi_receive_byte(uint8_t drive);
bool sd_spi_receive_block(uint8_t drive, uint8_t *ptr, unsigned int length);
bool sd_spi_transmit_block(uint8_t drive, uint8_t *ptr, unsigned int length);

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
