#ifndef __RTC_REG_DOT_H__
#define __RTC_REG_DOT_H__

/*
 * rp5c01a
 *
 * Real Time Clock with internal non-volatile RAM
 *
 */

#include <kernel.h>

/* mode 00 registers */
#define REG_1_SEC_CTR	    0x0
#define REG_10_SEC_CTR	    0x1
#define REG_1_MIN_CTR	    0x2
#define REG_10_MIN_CTR	    0x3
#define REG_1_HOUR_CTR	    0x4
#define REG_10_HOUR_CTR	    0x5
#define REG_WKDAY_CTR	    0x6
#define REG_1_DAY_CTR	    0x7
#define REG_10DAY_CTR	    0x8
#define REG_1_MONTH_CTR	    0x9
#define REG_10_MONTH_CTR    0xA
#define REG_1_YEAR_CTR	    0xB
#define REG_10_YEAR_CTR	    0xC
/* common registers */
#define MODE_SEL	    0xD
#define TEST_SEL	    0xE
#define RESET		    0xF

/* MODE_SEL modes */
#define TIMER_ENABLE	    0x8
#define MODE00		    0x0	    /* set/read time */
#define MODE01		    0x1	    /* set/read alarm, 12/24 and leap yaer */
#define MODE10		    0x2	    /* read/write ram block 0 */
#define MODE11		    0x3	    /* read/write ram block 1 */

/* platform specific */
uint8_t rp5c01_read_reg(uint8_t reg);
void rp5c01_write_reg(uint8_t reg, uint8_t value);

/* read seconds */
uint8_t rp5c01_rtc_secs(void);

#ifdef CONFIG_RTC_REG_NVRAM
/* read/write non volatile ram */
void rp5c01_nvram_write(uint8_t addr, uint8_t value);
uint8_t rp5c01_nvram_read(uint8_t addr);
#endif

#endif
