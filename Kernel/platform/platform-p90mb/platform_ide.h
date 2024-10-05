#define IDE_IS_MMIO  1		/* MMIO IDE */

#define IDE_REG_DATA		0x01200000
#define IDE_REG_ERROR		0x01200001
#define IDE_REG_FEATURES	0x01200001
#define IDE_REG_SEC_COUNT	0x01200002
#define IDE_REG_LBA_0		0x01200003
#define IDE_REG_LBA_1		0x01200004
#define IDE_REG_LBA_2		0x01200005
#define IDE_REG_LBA_3		0x01200006
#define IDE_REG_DEVHEAD		0x01200006
#define IDE_REG_STATUS		0x01200007
#define IDE_REG_COMMAND		0x01200007

/* FIXME: Check altstatus/control mapping */

#define ide_select(x)
#define ide_deselect()

#define IDE_8BIT_ONLY

