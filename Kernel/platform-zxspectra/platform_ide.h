/*
 *	Simple 8bit IDE interface
 */

#define ide_select(x)
#define ide_deselect()

#define IDE_8BIT_ONLY

#define IDE_DRIVE_COUNT	2

#define IDE_REG_DATA		0x2B
#define IDE_REG_ERROR		0x2F
#define IDE_REG_FEATURES	0x2F
#define IDE_REG_SEC_COUNT	0x6B
#define IDE_REG_LBA_0		0x6F
#define IDE_REG_LBA_1		0xAB
#define IDE_REG_LBA_2		0xAF
#define IDE_REG_LBA_3		0xEB
#define IDE_REG_DEVHEAD		0xEB
#define IDE_REG_STATUS		0xEF
#define IDE_REG_COMMAND		0xEF

#define IDE_NONSTANDARD_XFER
