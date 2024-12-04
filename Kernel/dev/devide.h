#ifndef __DEVIDE_DOT_H__
#define __DEVIDE_DOT_H__

#include <kernel.h>
#include "platform_ide.h"

/* IDE Drive Configuration (in config.h)

   Define CONFIG_IDE if IDE hardware is present on your platform.

   Define IDE_DRIVE_COUNT to the number of IDE drives your hardware
   supports (at most 16) - defaults to 2 if undefined.

   Define IDE_8BIT_ONLY if the system implements only half of the 16-bit data
   bus (eg n8vem-mark4). You can define IDE_IS_8BIT(iface) if you have a mix
   of types. FIXME: we should rename IDE_8BIT_ONLY.

   Define IDE_REG_INDIRECT if the IDE registers are not directly addressable on
   your platform. If you do not define IDE_REG_INDIRECT then IDE registers
   should be directly addressable by CPU I/O operations.

   If IDE_REG_INDIRECT is defined you will need to provide devide_readb() and
   devide_writeb() to access the IDE registers. You will need to define
   suitable values for each register (ide_reg_data, ide_reg_error etc) to be
   passed to these functions. You will also need to provide devide_read_data()
   and devide_write_data() to transfer sectors. See zeta-v2's PPIDE device code
   for an example of how to do this.

   If IDE_REG_INDIRECT is not defined: If the IDE registers appear in one
   contiguous block then define IDE_REG_BASE and either IDE_REG_CS0_FIRST or
   IDE_REG_CS1_FIRST. If the IDE registers appear in two non-contiguous blocks
   then define both IDE_REG_CS0_BASE and IDE_REG_CS1_BASE. If neither of these
   is suitable just define the address of each register ie IDE_REG_DATA,
   IDE_REG_ERROR, etc.
*/

void devide_init(void);

#ifdef IDE_REG_INDIRECT
uint8_t devide_readb(uint_fast8_t regaddr);
void devide_writeb(uint_fast8_t regaddr, uint_fast8_t value);
#else /* not IDE_REG_INDIRECT */
#define devide_readb(r)        (r)
#define devide_writeb(r,v)     do { r = v; } while(0)

#ifdef IDE_REG_BASE
#ifdef IDE_REG_CS0_FIRST
#define IDE_REG_CS0_BASE   (IDE_REG_BASE+0x00)
#define IDE_REG_CS1_BASE   (IDE_REG_BASE+0x08)
#endif
#ifdef IDE_REG_CS1_FIRST
#define IDE_REG_CS0_BASE   (IDE_REG_BASE+0x08)
#define IDE_REG_CS1_BASE   (IDE_REG_BASE+0x00)
#endif
#endif /* IDE_REG_BASE */

#ifdef IDE_REG_CS0_BASE
#define IDE_REG_ALTSTATUS (IDE_REG_CS0_BASE + 0x06) 
#define IDE_REG_CONTROL   (IDE_REG_CS0_BASE + 0x06) 
#endif

#ifdef IDE_REG_CS1_BASE
#define IDE_REG_DATA      (IDE_REG_CS1_BASE + 0x00) 
#define IDE_REG_ERROR     (IDE_REG_CS1_BASE + 0x01) 
#define IDE_REG_FEATURES  (IDE_REG_CS1_BASE + 0x01) 
#define IDE_REG_SEC_COUNT (IDE_REG_CS1_BASE + 0x02) 
#define IDE_REG_LBA_0     (IDE_REG_CS1_BASE + 0x03) 
#define IDE_REG_LBA_1     (IDE_REG_CS1_BASE + 0x04) 
#define IDE_REG_LBA_2     (IDE_REG_CS1_BASE + 0x05) 
#define IDE_REG_LBA_3     (IDE_REG_CS1_BASE + 0x06) 
#define IDE_REG_DEVHEAD   (IDE_REG_CS1_BASE + 0x06) 
#define IDE_REG_STATUS    (IDE_REG_CS1_BASE + 0x07) 
#define IDE_REG_COMMAND   (IDE_REG_CS1_BASE + 0x07) 
#endif

#endif /* IDE_REG_INDIRECT */

/* IDE status register bits */
#define IDE_STATUS_BUSY         0x80
#define IDE_STATUS_READY        0x40
#define IDE_STATUS_DEVFAULT     0x20
#define IDE_STATUS_SEEKCOMPLETE 0x10 /* not important */
#define IDE_STATUS_DATAREQUEST  0x08
#define IDE_STATUS_CORRECTED    0x04 /* not important */
#define IDE_STATUS_INDEX        0x02 /* not important */
#define IDE_STATUS_ERROR        0x01

/* IDE command codes */
#define IDE_CMD_READ_SECTOR     0x20
#define IDE_CMD_WRITE_SECTOR    0x30
#define IDE_CMD_INIT_DEV_PARAM	0x91	/* Needed for CHS drives */
#define IDE_CMD_FLUSH_CACHE     0xE7
#define IDE_CMD_IDENTIFY        0xEC
#define IDE_CMD_SET_FEATURES    0xEF

#ifndef IDE_DRIVE_NR_MASK
#define IDE_DRIVE_NR_MASK    0x0F   /* low bit used to select master/slave */
#endif

#ifdef _IDE_PRIVATE

#ifndef IDE_DRIVE_COUNT
#define IDE_DRIVE_COUNT 2       /* at most 16 drives without adjusting IDE_DRIVE_NR_MASK */
#endif

/* Only used when 8BIT_ONLY defined */
#ifndef IDE_IS_8BIT
#define IDE_IS_8BIT(x)	(1)
#endif

/* we use the bits in the driver_data field of blkdev_t as follows: */
#define FLAG_CACHE_DIRTY 0x40
#define FLAG_WRITE_CACHE 0x80

extern bool devide_wait(uint_fast8_t bits);
extern uint_fast8_t devide_transfer_sector(void);
extern int devide_flush_cache(void);

/* Platform provided, or may be defaults */
extern void devide_write_data(void);
extern void devide_read_data(void);

/* Only present in CHS mode */
extern uint8_t ide_spt[IDE_DRIVE_COUNT];
extern uint8_t ide_heads[IDE_DRIVE_COUNT];
extern uint16_t ide_cyls[IDE_DRIVE_COUNT];

#ifndef IDE_REG_INDIRECT
#ifdef IDE_IS_MMIO
#define ide_reg_control *((volatile uint8_t *)IDE_REG_CONTROL)
#define ide_reg_command *((volatile uint8_t *)IDE_REG_COMMAND)
#define ide_reg_data 	*((volatile uint8_t *)IDE_REG_DATA)
#define ide_reg_devhead *((volatile uint8_t *)IDE_REG_DEVHEAD)
#define ide_reg_error	*((volatile uint8_t *)IDE_REG_ERROR)
#define ide_reg_features *((volatile uint8_t *)IDE_REG_FEATURES)
#define ide_reg_lba_0	*((volatile uint8_t *)IDE_REG_LBA_0)
#define ide_reg_lba_1	*((volatile uint8_t *)IDE_REG_LBA_1)
#define ide_reg_lba_2	*((volatile uint8_t *)IDE_REG_LBA_2)
#define ide_reg_lba_3	*((volatile uint8_t *)IDE_REG_LBA_3)
#define ide_reg_sec_count *((volatile uint8_t *)IDE_REG_SEC_COUNT)
#define ide_reg_status	*((volatile uint8_t *)IDE_REG_STATUS)

#else /* !MMIO */

#ifndef IDE_SFR
#define IDE_SFR __sfr
#endif

#ifdef IDE_REG_ALTSTATUS
IDE_SFR __at IDE_REG_ALTSTATUS ide_reg_altstatus;
#endif
#ifdef IDE_REG_CONTROL
IDE_SFR __at IDE_REG_CONTROL   ide_reg_control;
#endif
IDE_SFR __at IDE_REG_COMMAND   ide_reg_command;
IDE_SFR __at IDE_REG_DATA      ide_reg_data;
IDE_SFR __at IDE_REG_DEVHEAD   ide_reg_devhead;
IDE_SFR __at IDE_REG_ERROR     ide_reg_error;
IDE_SFR __at IDE_REG_FEATURES  ide_reg_features;
IDE_SFR __at IDE_REG_LBA_0     ide_reg_lba_0;
IDE_SFR __at IDE_REG_LBA_1     ide_reg_lba_1;
IDE_SFR __at IDE_REG_LBA_2     ide_reg_lba_2;
IDE_SFR __at IDE_REG_LBA_3     ide_reg_lba_3;
IDE_SFR __at IDE_REG_SEC_COUNT ide_reg_sec_count;
IDE_SFR __at IDE_REG_STATUS    ide_reg_status;
#endif /* MMIO */
#endif /* IDE_REG_INDIRECT */
#endif /* IDE_PRIVATE */
#endif
