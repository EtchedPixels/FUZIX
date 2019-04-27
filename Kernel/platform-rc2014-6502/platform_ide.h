#define ide_select(x)
#define ide_deselect()

#define IDE_IS_MMIO
#define IDE_8BIT_ONLY

#define IDE_REG_DATA		0xC010
#define IDE_REG_ERROR		0xC011
#define IDE_REG_FEATURES	0xC011
#define IDE_REG_SEC_COUNT	0xC012
#define IDE_REG_LBA_0		0xC013
#define IDE_REG_LBA_1		0xC014
#define IDE_REG_LBA_2		0xC015
#define IDE_REG_LBA_3		0xC016
#define IDE_REG_DEVHEAD		0xC016
#define IDE_REG_COMMAND		0xC017
#define IDE_REG_STATUS		0xC017
