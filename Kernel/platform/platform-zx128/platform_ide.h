/* Simple IDE interface */
#define IDE_REG_DATA		0xA3
#define IDE_REG_ERROR		0xA7
#define IDE_REG_FEATURES	0xA7
#define IDE_REG_SEC_COUNT	0xAB
#define IDE_REG_LBA_0		0xAF
#define IDE_REG_LBA_1		0xB3
#define IDE_REG_LBA_2		0xB7
#define IDE_REG_LBA_3		0xBB
#define IDE_REG_DEVHEAD		0xBB
#define IDE_REG_STATUS		0xBF
#define IDE_REG_COMMAND		0xBF

#define ide_select(x)
#define ide_deselect()
