#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x00F00020
#define IDE_REG_ERROR		0x00F00022
#define IDE_REG_FEATURES	0x00F00022
#define IDE_REG_SEC_COUNT	0x00F00024
#define IDE_REG_LBA_0		0x00F00026
#define IDE_REG_LBA_1		0x00F00028
#define IDE_REG_LBA_2		0x00F0002A
#define IDE_REG_LBA_3		0x00F0002C
#define IDE_REG_DEVHEAD		0x00F0002C
#define IDE_REG_STATUS		0x00F0002E
#define IDE_REG_COMMAND		0x00F0002E

#define ide_select(x)
#define ide_deselect()

