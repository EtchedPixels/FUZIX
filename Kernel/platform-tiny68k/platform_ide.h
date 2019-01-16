#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x00F01000
#define IDE_REG_ERROR		0x00F01003
#define IDE_REG_FEATURES	0x00F01003
#define IDE_REG_SEC_COUNT	0x00F01005
#define IDE_REG_LBA_0		0x00F01007
#define IDE_REG_LBA_1		0x00F01009
#define IDE_REG_LBA_2		0x00F0100B
#define IDE_REG_LBA_3		0x00F0100D
#define IDE_REG_DEVHEAD		0x00F0100D
#define IDE_REG_STATUS		0x00F0100F
#define IDE_REG_COMMAND		0x00F0100F

/* FIXME: Check altstatus/control mapping */

#define ide_select(x)
#define ide_deselect()
