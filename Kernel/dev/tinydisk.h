/*
 *	Tiny disk driver layer for small systems. Provides the basic blk functionality
 *	but in a lot less space and with limtiations
 */

#ifndef _TINYDISK_H
#define _TINYDISK_H

int td_open(uint_fast8_t minor, uint16_t flag);
int td_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int td_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int td_ioctl(uint_fast8_t minor, uarg_t request, char *data);

typedef int (*td_xfer)(uint_fast8_t unit, bool is_read, uint32_t block, uint8_t * dptr);
typedef int (*td_ioc)(uint_fast8_t unit, uarg_t request, char *data);
extern int td_ioctl_none(uint_fast8_t minor, uarg_t request, char *data);

/* Optional target provided function with CONFIG_TD_CUSTOMPART to handle platforms
   that don't use PC format tables */
extern uint_fast8_t td_plt_setup(uint_fast8_t unit, uint32_t *lba, void *br);
extern uint8_t td_page;
extern uint8_t td_raw;

/* Base disk and four partitions 0, 1-4. Can be overridden */
#ifndef CONFIG_TD_MAX_PART
#define CONFIG_TD_MAX_PART	4
#endif

#ifdef _TINYDISK_PRIVATE

extern uint32_t td_lba[CONFIG_TD_NUM][CONFIG_TD_MAX_PART + 1];
extern td_xfer td_op[CONFIG_TD_NUM];
extern td_ioc td_iop[CONFIG_TD_NUM];
extern uint8_t td_unit[CONFIG_TD_NUM];
#endif

/* Setup/discard time */
int td_register(uint_fast8_t unit, td_xfer rwop, td_ioc ioctl, uint_fast8_t parts);

#endif
