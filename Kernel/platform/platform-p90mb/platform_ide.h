#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x12000000
#define IDE_REG_ERROR		0x12000001
#define IDE_REG_FEATURES	0x12000001
#define IDE_REG_SEC_COUNT	0x12000002
#define IDE_REG_LBA_0		0x12000003
#define IDE_REG_LBA_1		0x12000004
#define IDE_REG_LBA_2		0x12000005
#define IDE_REG_LBA_3		0x12000006
#define IDE_REG_DEVHEAD		0x12000006
#define IDE_REG_STATUS		0x12000007
#define IDE_REG_COMMAND		0x12000007

/* FIXME: Check altstatus/control mapping */

#define ide_select(x)
#define ide_deselect()

#define IDE_8BIT_ONLY

