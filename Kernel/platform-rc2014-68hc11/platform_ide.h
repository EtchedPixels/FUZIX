#define ide_select(x)
#define ide_deselect()

#define IDE_IS_MMIO
#define IDE_8BIT_ONLY

#define IDE_REG_DATA		0xFE10
#define IDE_REG_ERROR		0xFE11
#define IDE_REG_FEATURES	0xFE11
#define IDE_REG_SEC_COUNT	0xFE12
#define IDE_REG_LBA_0		0xFE13
#define IDE_REG_LBA_1		0xFE14
#define IDE_REG_LBA_2		0xFE15
#define IDE_REG_LBA_3		0xFE16
#define IDE_REG_DEVHEAD		0xFE16
#define IDE_REG_COMMAND		0xFE17
#define IDE_REG_STATUS		0xFE17
