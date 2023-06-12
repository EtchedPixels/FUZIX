#ifndef __DEVSD_DOT_H__
#define __DEVSD_DOT_H__

/* SD Configuration (in config.h)

   Define CONFIG_SD if SD hardware is present on your platform.

   Define SD_DRIVE_COUNT to the number of SD cards your hardware
   supports (at most 16)

   Provide the platform-specific SPI functions listed below to
   drive the SPI bus on your hardware, for an example of how to
   do this see platform-n8vem-mark4/devsdspi.c

   The required functions are:

    - sd_spi_clock(): switch between a slow clock (100--400kHz) for
    initialisation, and a fast (up to 20--25MHz) for normal operation.

    - sd_spi_raise_cs(), sd_spi_lower_cs(): raise or lower the CS line.

    - sd_spi_transmit_byte(): transmit a single byte

    - sd_spi_receive_byte(): receive a single byte

    - sd_spi_transmit_sector(): transmit a 512-byte sector (params in blk_op)

    - sd_spi_receive_sector(): receive a 512-byte sector (params in blk_op)
*/


/* So systems can fastcall or asm call these */

#ifndef SD_SPI_CALLTYPE
#define SD_SPI_CALLTYPE
#endif

/* public interface */
void devsd_init(void);

/* platform-specific SPI functions */
void sd_spi_clock(bool go_fast) SD_SPI_CALLTYPE;
void sd_spi_raise_cs(void);
void sd_spi_lower_cs(void);
void sd_spi_transmit_byte(uint_fast8_t byte) SD_SPI_CALLTYPE;
uint_fast8_t sd_spi_receive_byte(void);

bool sd_spi_receive_sector(void);
bool sd_spi_transmit_sector(void);

uint_fast8_t sd_spi_try_release(void);

/* for platforms which support multiple SD cards */
extern uint_fast8_t sd_drive; /* current card/drive number */

#ifdef _SD_PRIVATE

/* internal functions */
void sd_spi_release(void);
int sd_send_command(uint_fast8_t cmd, uint32_t arg);
uint_fast8_t sd_spi_wait(bool want_ff);
void sd_init_drive(uint8_t drive);
int sd_spi_init(void);
uint_fast8_t devsd_transfer_sector(void);

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

/* Use the top four bits of driver_data field of blkdev_t for the card type */
#define CT_NONE  0x00
#define CT_MMC   0x10
#define CT_SD1   0x20
#define CT_SD2   0x40
#define CT_BLOCK 0x80 /* set if block addressed, unset if byte addressed */
#define CT_SDC   (CT_SD1|CT_SD2)

/* Low four bits of driver_data are available to store drive number */
#define DRIVE_NR_MASK 0x0F

#define SD_DRIVE_NONE 0xFF /* When sd_drive is set to this there is no sd disk transaction on the bus.
			      You *MUST* set CONFIG_SPI_SHARED for this to be useful */
#endif

#endif /* __DEVSD_DOT_H__ */
