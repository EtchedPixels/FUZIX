/*
 *	DivIDE interface
 *
 *	This is a 16bit interface with a latched data port. Each read
 *	from A3 fetches a word then returns low then high etc. In the other
 *	direction it latches then writes.
 *
 *	The latch is reset to the first state by any other port access in the
 *	IDE space (so the command write sets it up nicely for us)
 */

#define ide_select(x)
#define ide_deselect()

#define IDE_DRIVE_COUNT	2

#define IDE_REG_DATA		0xFD08
#define IDE_REG_ERROR		0xFD09
#define IDE_REG_FEATURES	0xFD09
#define IDE_REG_SEC_COUNT	0xFD0A
#define IDE_REG_LBA_0		0xFD0B
#define IDE_REG_LBA_1		0xFD0C
#define IDE_REG_LBA_2		0xFD0D
//#define IDE_REG_LBA_3		0xFD0E
#define IDE_REG_DEVHEAD		0xFD0E
#define IDE_REG_STATUS		0xFD0F
#define IDE_REG_COMMAND		0xFD0F

#define IDE_NONSTANDARD_XFER
