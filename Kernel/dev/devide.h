#ifndef __DEVIDE_DOT_H__
#define __DEVIDE_DOT_H__

#include "config.h"

/* IDE Drive Configuration (in config.h)
 
   Define DEVICE_IDE if IDE hardware is present on your platform.

   Define IDE_8BIT_ONLY if the system implements only half of the 16-bit data
   bus (eg N8VEM Mark IV).

   If the IDE registers appear in one contiguous block then define IDE_REG_BASE
   and either IDE_REG_CS0_FIRST or IDE_REG_CS1_FIRST.

   If the IDE registers appear in two non-contiguous blocks then define both
   IDE_REG_CS0_BASE and IDE_REG_CS1_BASE.

   If neither of these is suitable just define the address of each register
   ie IDE_REG_DATA, IDE_REG_ERROR, etc.
*/

void devide_init(void);

#ifdef IDE_REG_BASE
#ifdef IDE_REG_CS0_FIRST
#define IDE_REG_CS0_BASE   (IDE_REG_BASE+0x00)
#define IDE_REG_CS1_BASE   (IDE_REG_BASE+0x08)
#endif
#ifdef IDE_REG_CS1_FIRST
#define IDE_REG_CS0_BASE   (IDE_REG_BASE+0x08)
#define IDE_REG_CS1_BASE   (IDE_REG_BASE+0x00)
#endif
#endif

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

/* IDE status register bits */
#define IDE_STATUS_BUSY         0x80
#define IDE_STATUS_READY        0x40
#define IDE_STATUS_DEVFAULT     0x20
#define IDE_STATUS_SEEKCOMPLETE 0x10 // not important
#define IDE_STATUS_DATAREQUEST  0x08
#define IDE_STATUS_CORRECTED    0x04 // not important
#define IDE_STATUS_INDEX        0x02 // not important
#define IDE_STATUS_ERROR        0x01

/* IDE command codes */
#define IDE_CMD_READ_SECTOR     0x20
#define IDE_CMD_WRITE_SECTOR    0x30
#define IDE_CMD_IDENTIFY        0xEC
#define IDE_CMD_SET_FEATURES    0xEF

#endif
