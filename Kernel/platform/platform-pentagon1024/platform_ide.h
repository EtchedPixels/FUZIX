/*
 *	NemoIDE interface
 *
 *	This is a 16bit interface with a latched data port.
 *
 *	The latch means we have to work from two ports which is ugly but
 *	at least it's not using the high bits of the address.
 */

#define ide_select(x)
#define ide_deselect()

#define IDE_DRIVE_COUNT	2

#define IDE_REG_DATA		0x10
#define IDE_REG_DATA_LATCH	0x11
#define IDE_REG_ERROR		0x30
#define IDE_REG_FEATURES	0x30
#define IDE_REG_SEC_COUNT	0x50
#define IDE_REG_LBA_0		0x70
#define IDE_REG_LBA_1		0x90
#define IDE_REG_LBA_2		0xB0
#define IDE_REG_LBA_3		0xD0
#define IDE_REG_DEVHEAD		0xD0
#define IDE_REG_STATUS		0xF0
#define IDE_REG_COMMAND		0xF0

#define IDE_REG_CONTROL		0xC8

#define IDE_NONSTANDARD_XFER
