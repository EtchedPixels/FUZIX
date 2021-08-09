#ifndef ROM_H
#define ROM_H

/* Uses no system RAM */
extern void uart_div_modify(uint8_t uart_no, uint32_t divlatch);
extern void Cache_Read_Enable(uint8_t odd_even, uint8_t mb_count, uint8_t no_idea);
extern void Cache_Read_Disable(void);

/* These routines use a 24 byte table pointed to by a pointer at 3FFFC714
   Unless touched the table points to the next 6 words (3FFC718 onwards). We need
   to preserve this table in order to use the SPI routines, and we don't want to
   duplicate these as they must live in precious IRAM if we do */
extern void SpiWrite(uint32_t address, const uint8_t *buffer, uint32_t length);
extern void SpiRead(uint32_t address, uint8_t *buffer, uint32_t length);
extern void SpiUnlock(void);
extern void SpiEraseSector(uint32_t sector);
extern void Wait_SPI_Idle(void* flash);
extern void SpiFlashCnfig(uint32_t spi_interface, uint32_t spi_freq);
extern int SpiReadModeCnfig(uint32_t mode);
/* The table pointer itself */
extern void* sdk_flashchip;

/* And we don't care what this one uses */
extern void system_restart(void);

#define ETS_SLC_INUM        1
#define ETS_SDIO_INUM       1
#define ETS_SPI_INUM        2
#define ETS_GPIO_INUM       4
#define ETS_UART_INUM       5
#define ETS_UART1_INUM      5
#define ETS_CCOMPARE0_INUM  6
#define ETS_SOFT_INUM       7
#define ETS_WDT_INUM        8
#define ETS_FRC_TIMER1_INUM 9  /* use edge */

/* Our exception frame: needs moving from rom.h as it's no longer the ROM one */
struct exception_frame {
	/* Saved in the fault handler */
	uint32_t a0;
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;
	uint32_t a8;
	uint32_t a9;
	uint32_t a10;
	uint32_t a11;
	uint32_t spare;
	uint32_t a14;
	uint32_t a15;
	uint32_t sar;
	uint32_t epc1;
	uint32_t exccause;
	uint32_t excvaddr;
	/* The registers saved in the initial stub */
	uint32_t a12;
	uint32_t a13;
};
#endif

/* vim: sw=4 ts=4 et: */

