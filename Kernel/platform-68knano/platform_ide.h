#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x900000	/* 16bit */
#define IDE_REG_ERROR		0x900003
#define IDE_REG_FEATURES	0x900003
#define IDE_REG_SEC_COUNT	0x900005
#define IDE_REG_LBA_0		0x900007
#define IDE_REG_LBA_1		0x900009
#define IDE_REG_LBA_2		0x90000B
#define IDE_REG_LBA_3		0x90000D
#define IDE_REG_DEVHEAD		0x90000D
#define IDE_REG_STATUS		0x90000F
#define IDE_REG_COMMAND		0x90000F

#define ide_select(x)
#define ide_deselect()
