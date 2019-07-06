#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x00FFE000
#define IDE_REG_ERROR		0x00FFE003
#define IDE_REG_FEATURES	0x00FFE003
#define IDE_REG_SEC_COUNT	0x00FFE005
#define IDE_REG_LBA_0		0x00FFE007
#define IDE_REG_LBA_1		0x00FFE009
#define IDE_REG_LBA_2		0x00FFE00B
#define IDE_REG_LBA_3		0x00FFE00D
#define IDE_REG_DEVHEAD		0x00FFE00D
#define IDE_REG_STATUS		0x00FFE00F
#define IDE_REG_COMMAND		0x00FFE00F

/* FIXME: Check altstatus/control mapping */

#define ide_select(x)
#define ide_deselect()

#define CONFIG_IDE_BSWAP
