#ifndef TINYSD_H
#define TINYSD_H

#include <stdbool.h>

extern uint_fast8_t sd_shift[CONFIG_TD_NUM];
extern uint_fast8_t sd_dev[CONFIG_TD_NUM];
extern uint8_t tinysd_busy;
extern uint8_t tinysd_unit;

int sd_xfer(uint_fast8_t unit, bool is_read, uint32_t lba, uint8_t * dptr);
uint8_t sd_init(uint_fast8_t unit);
void sd_probe(void);

/* Helpers */
#ifndef SD_SPI_CALLTYPE
#define SD_SPI_CALLTYPE
#endif

/* platform-specific SPI functions - match devsd as close as we can */
void sd_spi_raise_cs(void);
void sd_spi_lower_cs(void);
void sd_spi_tx_byte(uint_fast8_t byte) SD_SPI_CALLTYPE;
uint_fast8_t sd_spi_rx_byte(void);
void sd_spi_slow(void);
void sd_spi_fast(void);

bool sd_spi_rx_sector(uint8_t * p) SD_SPI_CALLTYPE;
bool sd_spi_tx_sector(uint8_t * p) SD_SPI_CALLTYPE;

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)	/* GO_IDLE_STATE */
#define CMD1    (0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8    (0x40+8)	/* SEND_IF_COND */
#define CMD9    (0x40+9)	/* SEND_CSD */
#define CMD10   (0x40+10)	/* SEND_CID */
#define CMD12   (0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13  (0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16   (0x40+16)	/* SET_BLOCKLEN */
#define CMD17   (0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18   (0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23   (0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (0x40+24)	/* WRITE_BLOCK */
#define CMD25   (0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55   (0x40+55)	/* APP_CMD */
#define CMD58   (0x40+58)	/* READ_OCR */

#define CT_NONE  0x00
#define CT_MMC   0x10
#define CT_SD1   0x20
#define CT_SD2   0x40
#define CT_BLOCK 0x80 /* set if block addressed, unset if byte addressed */
#define CT_SDC   (CT_SD1|CT_SD2)

#endif
