/*
 *	ZX Evolution. Sort of a NEMO IDE but has extra magic so you don't
 *	have to play with two ports and sometimes byteswap but can just inir
 *	and otir.
 */

#define ide_select(x)
#define ide_deselect()

#define IDE_DRIVE_COUNT	2

#define IDE_REG_DATA		0x10
#define IDE_DATA_LATCH		0x11
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

#define IDE_REG_ALTSTATUS	0xC8
#define IDE_REG_CONTROL		0xC8

